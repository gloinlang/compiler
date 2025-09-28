#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(length + 1);
    fread(content, 1, length, file);
    content[length] = '\0';
    
    fclose(file);
    return content;
}

ASTNode *parse_file(const char *filename) {
    char *content = read_file(filename);
    if (!content) {
        return NULL;
    }
    
    Lexer *lexer = create_lexer(content);
    Parser *parser = create_parser(lexer);
    
    ASTNode *ast = parse_program(parser);
    
    // Clean up lexer and parser, but keep the AST
    free_parser(parser);
    free_lexer(lexer);
    free(content);
    
    return ast;
}

typedef struct {
    char *name;
    char *version;
    char *url;
} Dependency;

typedef struct {
    char *name;
    char *version;
    char *author;
    Dependency *dependencies;
    int dependency_count;
} ArmoryConfig;

ArmoryConfig *parse_armory_toml(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL; // armory.toml is optional
    }
    
    ArmoryConfig *config = malloc(sizeof(ArmoryConfig));
    memset(config, 0, sizeof(ArmoryConfig));
    config->dependencies = malloc(sizeof(Dependency) * 32); // Max 32 dependencies
    
    char line[512];
    char current_section[64] = "";
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline and trim whitespace
        char *end = line + strlen(line) - 1;
        while (end > line && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
            *end = '\0';
            end--;
        }
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        // Check for section headers [section]
        if (line[0] == '[' && line[strlen(line)-1] == ']') {
            strncpy(current_section, line + 1, strlen(line) - 2);
            current_section[strlen(line) - 2] = '\0';
            continue;
        }
        
        // Parse key = value pairs
        char *equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;
        
        // Trim spaces around key and value
        while (*key == ' ' || *key == '\t') key++;
        while (*value == ' ' || *value == '\t') value++;
        
        // Remove quotes from value if present
        if (value[0] == '"' && value[strlen(value)-1] == '"') {
            value++;
            value[strlen(value)-1] = '\0';
        }
        
        // Parse based on current section
        if (strcmp(current_section, "project") == 0) {
            if (strcmp(key, "name") == 0) {
                config->name = strdup(value);
            } else if (strcmp(key, "version") == 0) {
                config->version = strdup(value);
            } else if (strcmp(key, "author") == 0) {
                config->author = strdup(value);
            }
        } else if (strcmp(current_section, "dependencies") == 0) {
            // Parse dependency: name = { version = "1.0.0", url = "..." }
            if (config->dependency_count < 32) {
                Dependency *dep = &config->dependencies[config->dependency_count];
                dep->name = strdup(key);
                
                // Simple parsing for { version = "...", url = "..." }
                char *version_start = strstr(value, "version = \"");
                if (version_start) {
                    version_start += 11; // Skip 'version = "'
                    char *version_end = strchr(version_start, '"');
                    if (version_end) {
                        *version_end = '\0';
                        dep->version = strdup(version_start);
                        *version_end = '"'; // Restore for url parsing
                    }
                }
                
                char *url_start = strstr(value, "url = \"");
                if (url_start) {
                    url_start += 7; // Skip 'url = "'
                    char *url_end = strchr(url_start, '"');
                    if (url_end) {
                        *url_end = '\0';
                        dep->url = strdup(url_start);
                    }
                }
                
                config->dependency_count++;
            }
        }
    }
    
    fclose(file);
    return config;
}

void free_armory_config(ArmoryConfig *config) {
    if (!config) return;
    
    free(config->name);
    free(config->version);
    free(config->author);
    
    for (int i = 0; i < config->dependency_count; i++) {
        free(config->dependencies[i].name);
        free(config->dependencies[i].version);
        free(config->dependencies[i].url);
    }
    free(config->dependencies);
    free(config);
}

void print_ast_node(ASTNode *node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.program.import_count; i++) {
                print_ast_node(node->data.program.imports[i], indent + 1);
            }
            for (int i = 0; i < node->data.program.function_count; i++) {
                print_ast_node(node->data.program.functions[i], indent + 1);
            }
            break;
        case NODE_IMPORT:
            printf("Import: %s (type: %d)\n", node->data.import.path, node->data.import.import_type);
            break;
        case NODE_FUNCTION:
            printf("Function: %s -> %s\n", node->data.function.name, node->data.function.return_type);
            for (int i = 0; i < node->data.function.param_count; i++) {
                print_ast_node(node->data.function.params[i], indent + 1);
            }
            if (node->data.function.body) {
                print_ast_node(node->data.function.body, indent + 1);
            }
            break;
        case NODE_PARAMETER:
            printf("Parameter: %s: %s\n", node->data.parameter.name, node->data.parameter.type);
            break;
        case NODE_BLOCK:
            printf("Block\n");
            for (int i = 0; i < node->data.block.statement_count; i++) {
                print_ast_node(node->data.block.statements[i], indent + 1);
            }
            break;
        case NODE_VARIABLE_DECL:
            printf("Variable Declaration: %s%s: %s\n", 
                   node->data.variable_decl.is_mutable ? "mut " : "",
                   node->data.variable_decl.name, 
                   node->data.variable_decl.type);
            if (node->data.variable_decl.value) {
                print_ast_node(node->data.variable_decl.value, indent + 1);
            }
            break;
        case NODE_ASSIGNMENT:
            printf("Assignment: %s\n", node->data.assignment.variable_name);
            if (node->data.assignment.value) {
                print_ast_node(node->data.assignment.value, indent + 1);
            }
            break;
        case NODE_POINTER_ASSIGNMENT:
            printf("Pointer Assignment\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Target:\n");
            print_ast_node(node->data.pointer_assignment.target, indent + 1);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Value:\n");
            print_ast_node(node->data.pointer_assignment.value, indent + 1);
            break;
        case NODE_RETURN:
            printf("Return\n");
            if (node->data.return_stmt.value) {
                print_ast_node(node->data.return_stmt.value, indent + 1);
            }
            break;
        case NODE_CALL:
            printf("Call: %s\n", node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                print_ast_node(node->data.call.args[i], indent + 1);
            }
            break;
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier.name);
            break;
        case NODE_LITERAL:
            printf("Literal: %s (%s)\n", node->data.literal.value, node->data.literal.type);
            break;
        case NODE_BINARY_OP:
            printf("Binary Op: %s\n", 
                   node->data.binary_op.operator == OP_ADD ? "+" :
                   node->data.binary_op.operator == OP_SUB ? "-" :
                   node->data.binary_op.operator == OP_MUL ? "*" :
                   node->data.binary_op.operator == OP_DIV ? "/" :
                   node->data.binary_op.operator == OP_EQ ? "==" :
                   node->data.binary_op.operator == OP_NE ? "!=" :
                   node->data.binary_op.operator == OP_LT ? "<" :
                   node->data.binary_op.operator == OP_GT ? ">" :
                   node->data.binary_op.operator == OP_LE ? "<=" :
                   node->data.binary_op.operator == OP_GE ? ">=" : "?");
            print_ast_node(node->data.binary_op.left, indent + 1);
            print_ast_node(node->data.binary_op.right, indent + 1);
            break;
        case NODE_UNARY_OP:
            printf("Unary Op: %s\n", 
                   node->data.unary_op.operator == UNARY_ADDRESS_OF ? "&" :
                   node->data.unary_op.operator == UNARY_DEREFERENCE ? "*" : "?");
            print_ast_node(node->data.unary_op.operand, indent + 1);
            break;
        case NODE_STRUCT:
            printf("Struct: %s\n", node->data.struct_decl.name);
            for (int i = 0; i < node->data.struct_decl.field_count; i++) {
                print_ast_node(node->data.struct_decl.fields[i], indent + 1);
            }
            for (int i = 0; i < node->data.struct_decl.method_count; i++) {
                print_ast_node(node->data.struct_decl.methods[i], indent + 1);
            }
            break;
        case NODE_STRUCT_FIELD:
            printf("Field: %s: %s\n", node->data.struct_field.name, node->data.struct_field.type);
            break;
        case NODE_STRUCT_METHOD:
            printf("Method: %s %s -> %s\n", 
                   node->data.struct_method.visibility == VISIBILITY_PUBLIC ? "pub" : "priv",
                   node->data.struct_method.name, 
                   node->data.struct_method.return_type);
            for (int i = 0; i < node->data.struct_method.param_count; i++) {
                print_ast_node(node->data.struct_method.params[i], indent + 1);
            }
            if (node->data.struct_method.body) {
                print_ast_node(node->data.struct_method.body, indent + 1);
            }
            break;
        case NODE_FIELD_ACCESS:
            printf("Field Access: %s\n", node->data.field_access.field_name);
            print_ast_node(node->data.field_access.object, indent + 1);
            break;
        case NODE_METHOD_CALL:
            printf("Method Call: %s\n", node->data.method_call.method_name);
            printf("Object:\n");
            print_ast_node(node->data.method_call.object, indent + 1);
            for (int i = 0; i < node->data.method_call.arg_count; i++) {
                printf("Arg %d:\n", i);
                print_ast_node(node->data.method_call.args[i], indent + 1);
            }
            break;
        case NODE_STRUCT_LITERAL:
            printf("Struct Literal: %s\n", node->data.struct_literal.struct_type_name);
            for (int i = 0; i < node->data.struct_literal.field_count; i++) {
                printf("Field %s:\n", node->data.struct_literal.field_names[i]);
                print_ast_node(node->data.struct_literal.field_values[i], indent + 1);
            }
            break;
        case NODE_IF:
            printf("If Statement\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast_node(node->data.if_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Then:\n");
            print_ast_node(node->data.if_stmt.then_block, indent + 2);
            if (node->data.if_stmt.else_block) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Else:\n");
                print_ast_node(node->data.if_stmt.else_block, indent + 2);
            }
            break;
        case NODE_UNLESS:
            printf("Unless Statement\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast_node(node->data.unless_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Then:\n");
            print_ast_node(node->data.unless_stmt.then_block, indent + 2);
            if (node->data.unless_stmt.else_block) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Else:\n");
                print_ast_node(node->data.unless_stmt.else_block, indent + 2);
            }
            break;
        case NODE_FOR:
            printf("For Loop\n");
            if (node->data.for_stmt.init) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Init:\n");
                print_ast_node(node->data.for_stmt.init, indent + 2);
            }
            if (node->data.for_stmt.condition) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Condition:\n");
                print_ast_node(node->data.for_stmt.condition, indent + 2);
            }
            if (node->data.for_stmt.update) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Update:\n");
                print_ast_node(node->data.for_stmt.update, indent + 2);
            }
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            print_ast_node(node->data.for_stmt.body, indent + 2);
            break;
        case NODE_WHILE:
            printf("While Loop\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast_node(node->data.while_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            print_ast_node(node->data.while_stmt.body, indent + 2);
            break;
        case NODE_SWITCH:
            printf("Switch Statement\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Expression:\n");
            print_ast_node(node->data.switch_stmt.expression, indent + 2);
            for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("Case %d:\n", i);
                print_ast_node(node->data.switch_stmt.cases[i], indent + 2);
            }
            if (node->data.switch_stmt.default_case) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Default:\n");
                print_ast_node(node->data.switch_stmt.default_case, indent + 2);
            }
            break;
        case NODE_SWITCH_CASE:
            if (node->data.switch_case.value) {
                printf("Switch Case\n");
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Value:\n");
                print_ast_node(node->data.switch_case.value, indent + 2);
            } else {
                printf("Default Case\n");
            }
            for (int i = 0; i < node->data.switch_case.statement_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("Statement %d:\n", i);
                print_ast_node(node->data.switch_case.statements[i], indent + 2);
            }
            break;
        case NODE_MATCH:
            printf("Match Statement\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Expression:\n");
            print_ast_node(node->data.match_stmt.expression, indent + 2);
            for (int i = 0; i < node->data.match_stmt.case_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("Match Case %d:\n", i);
                print_ast_node(node->data.match_stmt.cases[i], indent + 2);
            }
            break;
        case NODE_MATCH_CASE:
            printf("Match Case\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Pattern:\n");
            print_ast_node(node->data.match_case.pattern, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            print_ast_node(node->data.match_case.body, indent + 2);
            break;
        case NODE_BREAK:
            printf("Break\n");
            break;
        case NODE_CONTINUE:
            printf("Continue\n");
            break;
    }
}

int init_project(const char *project_name) {
    // Create project directory if name provided
    if (project_name && strcmp(project_name, ".") != 0) {
        if (mkdir(project_name, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Error: Could not create project directory '%s'\n", project_name);
            return 1;
        }
        if (chdir(project_name) != 0) {
            fprintf(stderr, "Error: Could not enter project directory '%s'\n", project_name);
            return 1;
        }
    }
    
    // Create includes directory
    if (mkdir("includes", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create includes directory\n");
        return 1;
    }
    
    // Create main.gloin template
    FILE *main_file = fopen("main.gloin", "w");
    if (!main_file) {
        fprintf(stderr, "Error: Could not create main.gloin\n");
        return 1;
    }
    
    fprintf(main_file, "import \"@std\"\n\n");
    fprintf(main_file, "fn main() -> i32 {\n");
    fprintf(main_file, "    std.println(\"Hello, Gloin!\");\n");
    fprintf(main_file, "    return 0;\n");
    fprintf(main_file, "}\n");
    fclose(main_file);
    
    // Create armory.toml template
    FILE *armory_file = fopen("armory.toml", "w");
    if (!armory_file) {
        fprintf(stderr, "Error: Could not create armory.toml\n");
        return 1;
    }
    
    const char *project_display_name = project_name ? project_name : "gloin_project";
    fprintf(armory_file, "[project]\n");
    fprintf(armory_file, "name = \"%s\"\n", project_display_name);
    fprintf(armory_file, "version = \"0.1.0\"\n");
    fprintf(armory_file, "author = \"Your Name\"\n\n");
    fprintf(armory_file, "[dependencies]\n");
    fprintf(armory_file, "# Add your dependencies here\n");
    fprintf(armory_file, "# example_lib = { version = \"1.0.0\", url = \"github.com/user/example_lib\" }\n");
    fclose(armory_file);
    
    printf("Successfully initialized Gloin project!\n");
    printf("Created:\n");
    printf("  - main.gloin\n");
    printf("  - armory.toml\n");
    printf("  - includes/\n");
    printf("\nTo compile and run:\n");
    printf("  gloinc main.gloin && ./main\n");
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s init [project_name]           # Initialize new project\n", argv[0]);
        fprintf(stderr, "  %s <filename> [options] [out]    # Compile Gloin file\n", argv[0]);
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, "  --debug                          # Show AST, LLVM IR and compile\n");
        fprintf(stderr, "  --ast, --parse-only             # Show AST and LLVM IR without compiling\n");
        fprintf(stderr, "  -o, --output <name>             # Specify output executable name\n");
        fprintf(stderr, "\nExamples:\n");
        fprintf(stderr, "  %s main.gloin                   # Compile to './main'\n", argv[0]);
        fprintf(stderr, "  %s main.gloin -o myapp          # Compile to './myapp'\n", argv[0]);
        fprintf(stderr, "  %s main.gloin --debug           # Show details and compile\n", argv[0]);
        fprintf(stderr, "  %s main.gloin --ast             # Show AST and LLVM IR only\n", argv[0]);
        return 1;
    }
    
    // Handle init command
    if (strcmp(argv[1], "init") == 0) {
        const char *project_name = (argc > 2) ? argv[2] : ".";
        return init_project(project_name);
    }
    
    // Handle file compilation (original functionality)
    if (argc > 6) {
        fprintf(stderr, "Too many arguments\n");
        return 1;
    }
    
    char *input_file = argv[1];
    int debug_mode = 0;        // Show details and compile
    int ast_only_mode = 0;     // Show AST/LLVM IR but don't compile
    char *output_name = NULL;  // Will be derived from input file if not specified
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "--ast") == 0 || strcmp(argv[i], "--parse-only") == 0) {
            ast_only_mode = 1;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_name = argv[++i];
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return 1;
            }
        } else {
            // If no flag, treat as output name for backward compatibility
            if (!output_name) {
                output_name = strdup(argv[i]);
            } else {
                fprintf(stderr, "Error: Unknown argument '%s'\n", argv[i]);
                return 1;
            }
        }
    }
    
    // Derive output name from input file if not specified (and we're compiling)
    if (!output_name && !ast_only_mode) {
        output_name = strdup(input_file);
        
        // Remove .gloin extension if present
        char *dot = strrchr(output_name, '.');
        if (dot && strcmp(dot, ".gloin") == 0) {
            *dot = '\0';
        }
    }
    
    char *content = read_file(input_file);
    if (!content) {
        if (output_name && strchr(argv[1], '/')) {
            free(output_name);
        }
        return 1;
    }
    
    if (debug_mode || ast_only_mode) {
        printf("Parsing file: %s\n", input_file);
        printf("Content:\n%s\n", content);
        printf("---\n");
    }
    
    Lexer *lexer = create_lexer(content);
    Parser *parser = create_parser(lexer);
    
    ASTNode *ast = parse_program(parser);
    
    if (debug_mode || ast_only_mode) {
        printf("AST:\n");
        print_ast_node(ast, 0);
        printf("\n");
    }
    
    // Always compile (unless there were parsing errors)
    if (!ast) {
        fprintf(stderr, "Compilation failed: parsing errors\n");
        free_parser(parser);
        free_lexer(lexer);
        free(content);
        if (output_name && strchr(argv[1], '/')) {
            free(output_name);
        }
        return 1;
    }
    
    if (debug_mode || ast_only_mode) {
        printf("Generating LLVM IR...\n");
    }
    
    // Create code generator
    CodeGen *codegen = create_codegen("gloin_module");
    
    // Generate code
    codegen_program(codegen, ast);
    if (codegen->has_error) {
        fprintf(stderr, "Code generation failed\n");
        free_codegen(codegen);
        free_ast_node(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(content);
        if (output_name && strchr(argv[1], '/')) {
            free(output_name);
        }
        return 1;
    }
    
    if (debug_mode || ast_only_mode) {
        printf("LLVM IR:\n");
        print_llvm_ir(codegen);
        printf("\n");
    }
    
    // Skip executable generation if in AST-only mode
    if (ast_only_mode) {
        printf("Parse completed successfully (no executable generated)\n");
        free_codegen(codegen);
        free_ast_node(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(content);
        return 0;
    }
    
    if (debug_mode) {
        printf("Generating executable: %s\n", output_name);
    }
    
    if (write_executable(codegen, output_name) == 0) {
        if (debug_mode) {
            printf("Successfully generated executable: %s\n", output_name);
        }
    } else {
        fprintf(stderr, "Failed to generate executable\n");
        free_codegen(codegen);
        free_ast_node(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(content);
        if (output_name && strchr(argv[1], '/')) {
            free(output_name);
        }
        return 1;
    }
    
    // Cleanup
    free_codegen(codegen);
    free_ast_node(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(content);
    if (output_name && strchr(argv[1], '/')) {
        free(output_name);
    }
    
    return 0;
}
