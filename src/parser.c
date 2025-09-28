#define _GNU_SOURCE
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *create_parser(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = next_token(lexer);
    return parser;
}

void free_parser(Parser *parser) {
    free_token(&parser->current_token);
    free(parser);
}

void eat(Parser *parser, TokenType expected_type) {
    if (parser->current_token.type == expected_type) {
        free_token(&parser->current_token);
        parser->current_token = next_token(parser->lexer);
    } else {
        fprintf(stderr, "Parser error: expected %s, got %s at line %d\n",
                token_type_to_string(expected_type),
                token_type_to_string(parser->current_token.type),
                parser->current_token.line);
        exit(1);
    }
}

void parser_error(Parser *parser, const char *message) {
    fprintf(stderr, "Parser error: %s at line %d, column %d\n",
            message, parser->current_token.line, parser->current_token.column);
    exit(1);
}

ASTNode *parse_program(Parser *parser) {
    ASTNode *program = create_program_node();
    
    // Skip initial newlines
    while (parser->current_token.type == TOKEN_NEWLINE) {
        eat(parser, TOKEN_NEWLINE);
    }
    
    while (parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_IMPORT) {
            ASTNode *import = parse_import(parser);
            add_import_to_program(program, import);
        } else if (parser->current_token.type == TOKEN_DEF) {
            // Parse def declarations: const, mut, functions, structs, enums
            eat(parser, TOKEN_DEF); // Consume DEF token first
            
            // Look at the next token to determine what kind of declaration this is
            if (parser->current_token.type == TOKEN_CONST || parser->current_token.type == TOKEN_MUT) {
                // Variable declaration: def const/mut name: type = value
                // Need to restore DEF token for parse_variable_declaration
                // Simple approach: create a new token and put it back
                Token def_token;
                def_token.type = TOKEN_DEF;
                def_token.value = strdup("def");
                def_token.line = parser->current_token.line;
                def_token.column = parser->current_token.column - 3; // "def" is 3 chars
                
                // We need to backup and let parse_variable_declaration handle it properly
                // For now, manually handle const/mut variable declarations here
                int is_const = (parser->current_token.type == TOKEN_CONST);
                eat(parser, parser->current_token.type); // eat const or mut
                
                if (parser->current_token.type != TOKEN_IDENTIFIER) {
                    parser_error(parser, "Expected variable name");
                }
                
                char *var_name = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);
                
                eat(parser, TOKEN_COLON);
                
                // Parse type
                char *var_type = NULL;
                if (parser->current_token.type == TOKEN_I32) {
                    var_type = strdup("i32");
                    eat(parser, TOKEN_I32);
                } else if (parser->current_token.type == TOKEN_F32) {
                    var_type = strdup("f32");
                    eat(parser, TOKEN_F32);
                } else if (parser->current_token.type == TOKEN_STRING_TYPE) {
                    var_type = strdup("string");
                    eat(parser, TOKEN_STRING_TYPE);
                } else if (parser->current_token.type == TOKEN_VOID) {
                    var_type = strdup("void");
                    eat(parser, TOKEN_VOID);
                } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
                    var_type = strdup(parser->current_token.value);
                    eat(parser, TOKEN_IDENTIFIER);
                } else {
                    parser_error(parser, "Expected variable type");
                }
                
                eat(parser, TOKEN_ASSIGN);
                ASTNode *value = parse_expression(parser);
                eat(parser, TOKEN_SEMICOLON);
                
                ASTNode *var_decl = create_variable_decl_node(var_name, var_type, value, is_const ? -1 : 1);
                add_function_to_program(program, var_decl);
                
                free(var_name);
                free(var_type);
                free_token(&def_token);
            } else if (parser->current_token.type == TOKEN_STRUCT) {
                // Struct declaration: def struct Name { ... }
                ASTNode *struct_decl = parse_struct_declaration(parser);
                add_function_to_program(program, struct_decl);
            } else if (parser->current_token.type == TOKEN_ENUM) {
                // Enum declaration: def enum Name { ... }
                ASTNode *enum_decl = parse_enum_declaration(parser);
                add_function_to_program(program, enum_decl);
            } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
                // Function declaration: def name(params) -> type { ... }
                ASTNode *function = parse_function_declaration(parser);
                add_function_to_program(program, function);
            } else {
                parser_error(parser, "Expected const, mut, struct, enum, or function name after 'def'");
            }
        } else if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
        } else {
            parser_error(parser, "Expected import or def declaration");
        }
    }
    
    return program;
}

ImportType parse_import_type(Parser *parser) {
    if (parser->current_token.type == TOKEN_AT) {
        eat(parser, TOKEN_AT);
        return IMPORT_STD;
    } else if (parser->current_token.type == TOKEN_HASH) {
        eat(parser, TOKEN_HASH);
        return IMPORT_EXTERNAL;
    } else if (parser->current_token.type == TOKEN_DOT) {
        eat(parser, TOKEN_DOT);
        eat(parser, TOKEN_DOT); // ./
        return IMPORT_LOCAL;
    } else {
        parser_error(parser, "Expected import type (@, #, or ./)");
        return IMPORT_STD; // Never reached
    }
}

ASTNode *parse_import(Parser *parser) {
    eat(parser, TOKEN_IMPORT);
    
    if (parser->current_token.type != TOKEN_STRING) {
        parser_error(parser, "Expected import path string");
        return NULL;
    }
    
    char *import_path = parser->current_token.value;
    ImportType type;
    char *clean_path;
    
    if (import_path[0] == '@') {
        type = IMPORT_STD;
        clean_path = strdup(import_path + 1); // Skip the @
    } else if (import_path[0] == '#') {
        type = IMPORT_EXTERNAL;
        clean_path = strdup(import_path + 1); // Skip the #
    } else if (strncmp(import_path, "./", 2) == 0) {
        type = IMPORT_LOCAL;
        clean_path = strdup(import_path + 2); // Skip the ./
    } else {
        parser_error(parser, "Invalid import path format");
        return NULL;
    }
    
    eat(parser, TOKEN_STRING);
    
    ASTNode *import = create_import_node(type, clean_path);
    free(clean_path);
    
    // Skip optional newline after import
    if (parser->current_token.type == TOKEN_NEWLINE) {
        eat(parser, TOKEN_NEWLINE);
    }
    
    return import;
}

ASTNode *parse_function_declaration(Parser *parser) {
    // Note: TOKEN_DEF already consumed by parse_program()
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected function name");
    }
    
    char *function_name = strdup(parser->current_token.value);
    eat(parser, TOKEN_IDENTIFIER);
    
    eat(parser, TOKEN_LPAREN);
    
    // First, collect parameters in temporary arrays
    char *param_names[256];
    char *param_types[256];
    int param_count = 0;
    
    // Parse parameters
    while (parser->current_token.type != TOKEN_RPAREN && parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parser_error(parser, "Expected parameter name");
        }
        
        // Get parameter name
        param_names[param_count] = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
        
        if (parser->current_token.type != TOKEN_COLON) {
            parser_error(parser, "Expected ':' after parameter name");
        }
        eat(parser, TOKEN_COLON);
        
        // Get parameter type (handle pointer types)
        if (parser->current_token.type == TOKEN_MULTIPLY) {
        eat(parser, TOKEN_MULTIPLY);
        // This is a pointer type
            if (parser->current_token.type == TOKEN_IDENTIFIER) {
            char *base_type = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
                char *ptr_type = malloc(strlen(base_type) + 2);
            sprintf(ptr_type, "*%s", base_type);
                param_types[param_count] = ptr_type;
            free(base_type);
        } else if (parser->current_token.type == TOKEN_BOOL) {
            param_types[param_count] = strdup("*bool");
            eat(parser, TOKEN_BOOL);
        } else {
            parser_error(parser, "Expected base type after '*'");
        }
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        param_types[param_count] = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
    } else if (parser->current_token.type == TOKEN_BOOL) {
        param_types[param_count] = strdup("bool");
        eat(parser, TOKEN_BOOL);
    } else if (parser->current_token.type == TOKEN_I32) {
        param_types[param_count] = strdup("i32");
        eat(parser, TOKEN_I32);
    } else if (parser->current_token.type == TOKEN_F32) {
        param_types[param_count] = strdup("f32");
        eat(parser, TOKEN_F32);
    } else if (parser->current_token.type == TOKEN_STRING_TYPE) {
        param_types[param_count] = strdup("string");
        eat(parser, TOKEN_STRING_TYPE);
    } else if (parser->current_token.type == TOKEN_VOID) {
        param_types[param_count] = strdup("void");
        eat(parser, TOKEN_VOID);
    } else {
        parser_error(parser, "Expected parameter type");
    }
        
        param_count++;
        
        // Check for comma (more parameters)
        if (parser->current_token.type == TOKEN_COMMA) {
            eat(parser, TOKEN_COMMA);
        } else if (parser->current_token.type != TOKEN_RPAREN) {
            parser_error(parser, "Expected ',' or ')' after parameter");
        }
    }
    
    eat(parser, TOKEN_RPAREN);
    
    eat(parser, TOKEN_ARROW);
    
    char *return_type;
    if (parser->current_token.type == TOKEN_MULTIPLY) {
        eat(parser, TOKEN_MULTIPLY);
        // This is a pointer type
        if (parser->current_token.type == TOKEN_IDENTIFIER) {
            char *base_type = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            return_type = malloc(strlen(base_type) + 2);
            sprintf(return_type, "*%s", base_type);
            free(base_type);
        } else if (parser->current_token.type == TOKEN_BOOL) {
            return_type = strdup("*bool");
            eat(parser, TOKEN_BOOL);
        } else {
            parser_error(parser, "Expected base type after '*'");
        }
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        return_type = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
    } else if (parser->current_token.type == TOKEN_BOOL) {
        return_type = strdup("bool");
        eat(parser, TOKEN_BOOL);
    } else if (parser->current_token.type == TOKEN_I32) {
        return_type = strdup("i32");
        eat(parser, TOKEN_I32);
    } else if (parser->current_token.type == TOKEN_F32) {
        return_type = strdup("f32");
        eat(parser, TOKEN_F32);
    } else if (parser->current_token.type == TOKEN_STRING_TYPE) {
        return_type = strdup("string");
        eat(parser, TOKEN_STRING_TYPE);
    } else if (parser->current_token.type == TOKEN_VOID) {
        return_type = strdup("void");
        eat(parser, TOKEN_VOID);
    } else {
        parser_error(parser, "Expected return type");
    }
    
    ASTNode *function = create_function_node(function_name, return_type);
    
    // Now add the parameters to the function
    for (int i = 0; i < param_count; i++) {
        ASTNode *param = create_parameter_node(param_names[i], param_types[i]);
        add_parameter_to_function(function, param);
        free(param_names[i]);
        free(param_types[i]);
    }
    
    function->data.function.body = parse_block(parser);
    
    free(function_name);
    free(return_type);
    
    return function;
}

ASTNode *parse_struct_declaration(Parser *parser) {
    // Note: TOKEN_DEF already consumed by parse_program()
    eat(parser, TOKEN_STRUCT); // struct keyword
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected struct name");
    }
    
    char *struct_name = strdup(parser->current_token.value);
    eat(parser, TOKEN_IDENTIFIER);
    
    eat(parser, TOKEN_LBRACE);
    
    ASTNode *struct_node = create_struct_node(struct_name);
    
    // Skip newlines after opening brace
    while (parser->current_token.type == TOKEN_NEWLINE) {
        eat(parser, TOKEN_NEWLINE);
    }
    
    while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
        } else if (parser->current_token.type == TOKEN_PUB || parser->current_token.type == TOKEN_PRIV) {
            // Parse method
            Visibility visibility = parser->current_token.type == TOKEN_PUB ? VISIBILITY_PUBLIC : VISIBILITY_PRIVATE;
            TokenType vis_token = parser->current_token.type;
            eat(parser, vis_token);  // eat pub or priv
            
            if (parser->current_token.type != TOKEN_IDENTIFIER) {
                parser_error(parser, "Expected method name after visibility modifier");
            }
            
            char *method_name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            eat(parser, TOKEN_LPAREN);
            
            ASTNode *method = create_struct_method_node(method_name, "void", visibility);
            
            // Parse parameters
            while (parser->current_token.type != TOKEN_RPAREN) {
                if (parser->current_token.type != TOKEN_IDENTIFIER) {
                    parser_error(parser, "Expected parameter name");
                }
                
                char *param_name = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);
                
                eat(parser, TOKEN_COLON);
                
                // Parse parameter type
                char *param_type;
                if (parser->current_token.type == TOKEN_MULTIPLY) {
                    eat(parser, TOKEN_MULTIPLY);
                    if (parser->current_token.type == TOKEN_IDENTIFIER) {
                        char *base_type = strdup(parser->current_token.value);
                        eat(parser, TOKEN_IDENTIFIER);
                        param_type = malloc(strlen(base_type) + 2);
                        sprintf(param_type, "*%s", base_type);
                        free(base_type);
                    } else {
                        parser_error(parser, "Expected type after '*'");
                        param_type = strdup("*unknown");
                    }
                } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
                    param_type = strdup(parser->current_token.value);
                    eat(parser, TOKEN_IDENTIFIER);
                } else if (parser->current_token.type == TOKEN_BOOL) {
                    param_type = strdup("bool");
                    eat(parser, TOKEN_BOOL);
                } else {
                    parser_error(parser, "Expected parameter type");
                    param_type = strdup("unknown");
                }
                
                ASTNode *param = create_parameter_node(param_name, param_type);
                add_parameter_to_struct_method(method, param);
                
                free(param_name);
                free(param_type);
                
                if (parser->current_token.type == TOKEN_COMMA) {
                    eat(parser, TOKEN_COMMA);
                } else if (parser->current_token.type != TOKEN_RPAREN) {
                    parser_error(parser, "Expected ',' or ')' in parameter list");
                }
            }
            
            eat(parser, TOKEN_RPAREN);
            
            // Parse return type
            if (parser->current_token.type == TOKEN_ARROW) {
                eat(parser, TOKEN_ARROW);
                
                if (parser->current_token.type == TOKEN_IDENTIFIER) {
                    free(method->data.struct_method.return_type);
                    method->data.struct_method.return_type = strdup(parser->current_token.value);
                    eat(parser, TOKEN_IDENTIFIER);
                } else if (parser->current_token.type == TOKEN_BOOL) {
                    free(method->data.struct_method.return_type);
                    method->data.struct_method.return_type = strdup("bool");
                    eat(parser, TOKEN_BOOL);
                } else {
                    parser_error(parser, "Expected return type after '->'");
                }
            }
            
            // Parse method body
            method->data.struct_method.body = parse_block(parser);
            
            add_method_to_struct(struct_node, method);
            free(method_name);
            
        } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
            // Parse field
            char *field_name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            eat(parser, TOKEN_COLON);
            
            // Parse field type
            char *field_type;
            if (parser->current_token.type == TOKEN_MULTIPLY) {
                eat(parser, TOKEN_MULTIPLY);
                if (parser->current_token.type == TOKEN_IDENTIFIER) {
                    char *base_type = strdup(parser->current_token.value);
                    eat(parser, TOKEN_IDENTIFIER);
                    field_type = malloc(strlen(base_type) + 2);
                    sprintf(field_type, "*%s", base_type);
                    free(base_type);
                } else {
                    parser_error(parser, "Expected type after '*'");
                    field_type = strdup("*unknown");
                }
            } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
                field_type = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);
            } else if (parser->current_token.type == TOKEN_BOOL) {
                field_type = strdup("bool");
                eat(parser, TOKEN_BOOL);
            } else {
                parser_error(parser, "Expected field type");
                field_type = strdup("unknown");
            }
            
            eat(parser, TOKEN_SEMICOLON);
            
            ASTNode *field = create_struct_field_node(field_name, field_type);
            add_field_to_struct(struct_node, field);
            
            free(field_name);
            free(field_type);
            
        } else {
            parser_error(parser, "Expected field or method declaration in struct");
        }
    }
    
    eat(parser, TOKEN_RBRACE);
    
    free(struct_name);
    return struct_node;
}

ASTNode *parse_enum_declaration(Parser *parser) {
    // Note: TOKEN_DEF already consumed by parse_program()
    eat(parser, TOKEN_ENUM);  // enum keyword
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected enum name");
    }
    
    char *enum_name = strdup(parser->current_token.value);
    eat(parser, TOKEN_IDENTIFIER);
    
    eat(parser, TOKEN_LBRACE);
    
    ASTNode *enum_node = create_enum_node(enum_name);
    
    // Skip newlines after opening brace
    while (parser->current_token.type == TOKEN_NEWLINE) {
        eat(parser, TOKEN_NEWLINE);
    }
    
    // Parse enum variants
    while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
            continue;
        }
        
        if (parser->current_token.type == TOKEN_IDENTIFIER) {
            char *variant_name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            ASTNode *variant = create_enum_variant_node(variant_name);
            add_enum_variant(enum_node, variant);
            free(variant_name);
            
            // Optional comma
            if (parser->current_token.type == TOKEN_COMMA) {
                eat(parser, TOKEN_COMMA);
            }
        } else {
            parser_error(parser, "Expected enum variant name");
        }
    }
    
    eat(parser, TOKEN_RBRACE);
    
    free(enum_name);
    return enum_node;
}

ASTNode *parse_block(Parser *parser) {
    eat(parser, TOKEN_LBRACE);
    
    ASTNode *block = create_block_node();
    
    // Skip newlines after opening brace
    while (parser->current_token.type == TOKEN_NEWLINE) {
        eat(parser, TOKEN_NEWLINE);
    }
    
    while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
            continue;
        }
        
        ASTNode *statement = parse_statement(parser);
        add_statement_to_block(block, statement);
    }
    
    eat(parser, TOKEN_RBRACE);
    
    return block;
}

ASTNode *parse_statement(Parser *parser) {
    if (parser->current_token.type == TOKEN_DEF) {
        return parse_variable_declaration(parser);
    } else if (parser->current_token.type == TOKEN_RETURN) {
        return parse_return_statement(parser);
    } else if (parser->current_token.type == TOKEN_IF) {
        return parse_if_statement(parser);
    } else if (parser->current_token.type == TOKEN_UNLESS) {
        return parse_unless_statement(parser);
    } else if (parser->current_token.type == TOKEN_FOR) {
        return parse_for_statement(parser);
    } else if (parser->current_token.type == TOKEN_WHILE) {
        return parse_while_statement(parser);
    } else if (parser->current_token.type == TOKEN_SWITCH) {
        return parse_switch_statement(parser);
    } else if (parser->current_token.type == TOKEN_MATCH) {
        return parse_match_statement(parser);
    } else if (parser->current_token.type == TOKEN_BREAK) {
        return parse_break_statement(parser);
    } else if (parser->current_token.type == TOKEN_CONTINUE) {
        return parse_continue_statement(parser);
    } else {
        // Try to parse as assignment first, if that fails, parse as expression
        return parse_expression_statement(parser);
    }
}

ASTNode *parse_variable_declaration(Parser *parser) {
    eat(parser, TOKEN_DEF);
    
    // Check for mutability modifiers
    int is_mutable = 0;  // Default: immutable
    if (parser->current_token.type == TOKEN_MUT) {
        is_mutable = 1;  // Mutable
        eat(parser, TOKEN_MUT);
    } else if (parser->current_token.type == TOKEN_CONST) {
        is_mutable = -1;  // Const
        eat(parser, TOKEN_CONST);
    }
    
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_error(parser, "Expected variable name");
    }
    
    char *var_name = strdup(parser->current_token.value);
    eat(parser, TOKEN_IDENTIFIER);
    
    eat(parser, TOKEN_COLON);
    
    char *var_type;
    if (parser->current_token.type == TOKEN_MULTIPLY) {
        eat(parser, TOKEN_MULTIPLY);
        // This is a pointer type
        if (parser->current_token.type == TOKEN_IDENTIFIER) {
            char *base_type = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            var_type = malloc(strlen(base_type) + 2);
            sprintf(var_type, "*%s", base_type);
            free(base_type);
        } else if (parser->current_token.type == TOKEN_BOOL) {
            var_type = strdup("*bool");
            eat(parser, TOKEN_BOOL);
        } else {
            parser_error(parser, "Expected base type after '*'");
            return NULL;
        }
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        var_type = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
    } else if (parser->current_token.type == TOKEN_BOOL) {
        var_type = strdup("bool");
        eat(parser, TOKEN_BOOL);
    } else if (parser->current_token.type == TOKEN_I32) {
        var_type = strdup("i32");
        eat(parser, TOKEN_I32);
    } else if (parser->current_token.type == TOKEN_F32) {
        var_type = strdup("f32");
        eat(parser, TOKEN_F32);
    } else if (parser->current_token.type == TOKEN_STRING_TYPE) {
        var_type = strdup("string");
        eat(parser, TOKEN_STRING_TYPE);
    } else if (parser->current_token.type == TOKEN_VOID) {
        var_type = strdup("void");
        eat(parser, TOKEN_VOID);
    } else {
        parser_error(parser, "Expected variable type");
        return NULL;
    }
    
    eat(parser, TOKEN_ASSIGN);
    
    ASTNode *value = parse_expression(parser);
    
    eat(parser, TOKEN_SEMICOLON);
    
    ASTNode *var_decl = create_variable_decl_node(var_name, var_type, value, is_mutable);
    
    free(var_name);
    free(var_type);
    
    return var_decl;
}

ASTNode *parse_return_statement(Parser *parser) {
    eat(parser, TOKEN_RETURN);
    
    ASTNode *value = NULL;
    if (parser->current_token.type != TOKEN_SEMICOLON) {
        value = parse_expression(parser);
    }
    
    eat(parser, TOKEN_SEMICOLON);
    
    return create_return_node(value);
}

ASTNode *parse_expression_statement(Parser *parser) {
    // Check if this is a pointer dereference assignment (*expr = expression)
    if (parser->current_token.type == TOKEN_MULTIPLY) {
        eat(parser, TOKEN_MULTIPLY);
        ASTNode *target = parse_primary(parser);
        
        if (parser->current_token.type == TOKEN_ASSIGN) {
            eat(parser, TOKEN_ASSIGN);
            ASTNode *value = parse_expression(parser);
            eat(parser, TOKEN_SEMICOLON);
            
            // Create a dereference node as the target of assignment
            ASTNode *deref_target = create_unary_op_node(UNARY_DEREFERENCE, target);
            return create_pointer_assignment_node(deref_target, value);
        } else {
            // Not an assignment, parse as regular dereference expression
            ASTNode *deref = create_unary_op_node(UNARY_DEREFERENCE, target);
            ASTNode *expr = parse_additive_continuation(parser, deref);
            eat(parser, TOKEN_SEMICOLON);
            return expr;
        }
    }
    // Check if this is an assignment statement (identifier = expression or *expr = expression)
    else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char *var_name = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
        
        if (parser->current_token.type == TOKEN_ASSIGN) {
            // This is a simple variable assignment
            eat(parser, TOKEN_ASSIGN);
            ASTNode *value = parse_expression(parser);
            eat(parser, TOKEN_SEMICOLON);
            
            ASTNode *assignment = create_assignment_node(var_name, value);
            free(var_name);
            return assignment;
        } else {
            // This is not an assignment, we need to continue parsing as expression
            // We've already consumed the identifier, so handle the rest
            ASTNode *left = create_identifier_node(var_name);
            free(var_name);
            
            // Check for function call
            if (parser->current_token.type == TOKEN_DOT) {
                eat(parser, TOKEN_DOT);
                if (parser->current_token.type != TOKEN_IDENTIFIER) {
                    parser_error(parser, "Expected method name after '.'");
                }
                char *method = strdup(parser->current_token.value);
                eat(parser, TOKEN_IDENTIFIER);
                
                if (parser->current_token.type == TOKEN_LPAREN) {
                    eat(parser, TOKEN_LPAREN);
                    
                    // Check if this is a std.* qualified function call
                    ASTNode *call;
                    if (strcmp(left->data.identifier.name, "std") == 0) {
                        // Create qualified function call
                        char *qualified_name = malloc(strlen(left->data.identifier.name) + strlen(method) + 2);
                        sprintf(qualified_name, "%s.%s", left->data.identifier.name, method);
                        call = create_call_node(qualified_name);
                        free(qualified_name);
                    } else {
                        // Regular method call
                        call = create_call_node(method);
                    }
                    
                    while (parser->current_token.type != TOKEN_RPAREN) {
                        ASTNode *arg = parse_expression(parser);
                        add_arg_to_call(call, arg);
                        
                        if (parser->current_token.type == TOKEN_COMMA) {
                            eat(parser, TOKEN_COMMA);
                        } else if (parser->current_token.type != TOKEN_RPAREN) {
                            parser_error(parser, "Expected ',' or ')' in function call");
                        }
                    }
                    eat(parser, TOKEN_RPAREN);
                    free(method);
                    free_ast_node(left);
                    eat(parser, TOKEN_SEMICOLON);
                    return call;
                } else {
                    // Not a method call, treat as field access (not implemented yet)
                    parser_error(parser, "Field access not implemented");
                    free(method);
                    free_ast_node(left);
                    return NULL;
                }
            } else if (parser->current_token.type == TOKEN_LPAREN) {
                // Direct function call on the identifier
                eat(parser, TOKEN_LPAREN);
                char *func_name = strdup(left->data.identifier.name);
                ASTNode *call = create_call_node(func_name);
                free(func_name);
                
                while (parser->current_token.type != TOKEN_RPAREN) {
                    ASTNode *arg = parse_expression(parser);
                    add_arg_to_call(call, arg);
                    
                    if (parser->current_token.type == TOKEN_COMMA) {
                        eat(parser, TOKEN_COMMA);
                    } else if (parser->current_token.type != TOKEN_RPAREN) {
                        parser_error(parser, "Expected ',' or ')' in function call");
                    }
                }
                eat(parser, TOKEN_RPAREN);
                free_ast_node(left);
                eat(parser, TOKEN_SEMICOLON);
                return call;
            } else {
                // Continue parsing potential binary operations
                ASTNode *expr = parse_additive_continuation(parser, left);
                eat(parser, TOKEN_SEMICOLON);
                return expr;
            }
        }
    } else {
        ASTNode *expr = parse_expression(parser);
        eat(parser, TOKEN_SEMICOLON);
        return expr;
    }
}

ASTNode *parse_if_statement(Parser *parser) {
    eat(parser, TOKEN_IF);
    
    // Parse condition (no parentheses)
    ASTNode *condition = parse_expression(parser);
    
    // Parse then block
    ASTNode *then_block = parse_block(parser);
    
    // Check for optional else block
    ASTNode *else_block = NULL;
    if (parser->current_token.type == TOKEN_ELSE) {
        eat(parser, TOKEN_ELSE);
        else_block = parse_block(parser);
    }
    
    return create_if_node(condition, then_block, else_block);
}

ASTNode *parse_unless_statement(Parser *parser) {
    eat(parser, TOKEN_UNLESS);
    
    // Parse condition (no parentheses)
    ASTNode *condition = parse_expression(parser);
    
    // Parse then block
    ASTNode *then_block = parse_block(parser);
    
    // Check for optional else block
    ASTNode *else_block = NULL;
    if (parser->current_token.type == TOKEN_ELSE) {
        eat(parser, TOKEN_ELSE);
        else_block = parse_block(parser);
    }
    
    return create_unless_node(condition, then_block, else_block);
}

ASTNode *parse_for_statement(Parser *parser) {
    eat(parser, TOKEN_FOR);
    
    // For now, let's implement a simple C-style for loop
    // for init; condition; update { body }
    
    ASTNode *init = NULL;
    ASTNode *condition = NULL;
    ASTNode *update = NULL;
    
    // Parse init (optional)
    if (parser->current_token.type != TOKEN_SEMICOLON) {
        if (parser->current_token.type == TOKEN_DEF) {
            // Parse variable declaration manually without consuming semicolon
            eat(parser, TOKEN_DEF);
            
            int is_mutable = 0;
            if (parser->current_token.type == TOKEN_MUT) {
                is_mutable = 1;
                eat(parser, TOKEN_MUT);
            }
            
            if (parser->current_token.type != TOKEN_IDENTIFIER) {
                parser_error(parser, "Expected variable name");
            }
            
            char *var_name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            eat(parser, TOKEN_COLON);
            
            char *var_type = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            eat(parser, TOKEN_ASSIGN);
            
            ASTNode *value = parse_expression(parser);
            
            init = create_variable_decl_node(var_name, var_type, value, is_mutable);
            
            free(var_name);
            free(var_type);
        } else {
            init = parse_expression(parser);
        }
    }
    eat(parser, TOKEN_SEMICOLON);
    
    // Parse condition (optional)
    if (parser->current_token.type != TOKEN_SEMICOLON) {
        condition = parse_expression(parser);
    }
    eat(parser, TOKEN_SEMICOLON);
    
    // Parse update (optional)
    if (parser->current_token.type != TOKEN_LBRACE) {
        // Check if this is an assignment
        if (parser->current_token.type == TOKEN_IDENTIFIER) {
            // Look ahead to see if this is an assignment
            char *var_name = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            if (parser->current_token.type == TOKEN_ASSIGN) {
                eat(parser, TOKEN_ASSIGN);
                ASTNode *value = parse_expression(parser);
                update = create_assignment_node(var_name, value);
            } else {
                // Not an assignment, put the identifier back and parse as expression
                // This is a limitation - we can't easily backtrack, so this won't work perfectly
                // For now, just create an identifier node
                update = create_identifier_node(var_name);
            }
            free(var_name);
        } else {
            update = parse_expression(parser);
        }
    }
    
    // Parse body
    ASTNode *body = parse_block(parser);
    
    return create_for_node(init, condition, update, body);
}

ASTNode *parse_while_statement(Parser *parser) {
    eat(parser, TOKEN_WHILE);
    
    // Parse condition (no parentheses)
    ASTNode *condition = parse_expression(parser);
    
    // Parse body
    ASTNode *body = parse_block(parser);
    
    return create_while_node(condition, body);
}

ASTNode *parse_switch_statement(Parser *parser) {
    eat(parser, TOKEN_SWITCH);
    
    // Parse expression to switch on
    ASTNode *expression = parse_expression(parser);
    
    eat(parser, TOKEN_LBRACE);
    
    ASTNode *switch_node = create_switch_node(expression);
    
    // Parse cases
    while (parser->current_token.type != TOKEN_RBRACE) {
        // Skip newlines
        if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
            continue;
        }
        
        if (parser->current_token.type == TOKEN_CASE) {
            eat(parser, TOKEN_CASE);
            
            // Parse case value
            ASTNode *case_value = parse_expression(parser);
            eat(parser, TOKEN_COLON);
            
            ASTNode *switch_case = create_switch_case_node(case_value);
            
            // Parse statements until next case or default or end
            while (parser->current_token.type != TOKEN_CASE && 
                   parser->current_token.type != TOKEN_DEFAULT && 
                   parser->current_token.type != TOKEN_RBRACE) {
                // Skip newlines within case statements
                if (parser->current_token.type == TOKEN_NEWLINE) {
                    eat(parser, TOKEN_NEWLINE);
                    continue;
                }
                ASTNode *statement = parse_statement(parser);
                add_statement_to_switch_case(switch_case, statement);
            }
            
            add_case_to_switch(switch_node, switch_case);
        } else if (parser->current_token.type == TOKEN_DEFAULT) {
            eat(parser, TOKEN_DEFAULT);
            eat(parser, TOKEN_COLON);
            
            ASTNode *default_case = create_switch_case_node(NULL); // NULL value for default
            
            // Parse statements until end
            while (parser->current_token.type != TOKEN_RBRACE) {
                // Skip newlines within default case
                if (parser->current_token.type == TOKEN_NEWLINE) {
                    eat(parser, TOKEN_NEWLINE);
                    continue;
                }
                ASTNode *statement = parse_statement(parser);
                add_statement_to_switch_case(default_case, statement);
            }
            
            set_switch_default(switch_node, default_case);
        } else {
            parser_error(parser, "Expected 'case' or 'default' in switch statement");
            break;
        }
    }
    
    eat(parser, TOKEN_RBRACE);
    
    return switch_node;
}

ASTNode *parse_match_statement(Parser *parser) {
    eat(parser, TOKEN_MATCH);
    
    // Parse expression to match on
    ASTNode *expression = parse_expression(parser);
    
    eat(parser, TOKEN_LBRACE);
    
    ASTNode *match_node = create_match_node(expression);
    
    // Parse match cases
    while (parser->current_token.type != TOKEN_RBRACE) {
        // Skip newlines
        if (parser->current_token.type == TOKEN_NEWLINE) {
            eat(parser, TOKEN_NEWLINE);
            continue;
        }
        
        ASTNode *pattern;
        
        if (parser->current_token.type == TOKEN_UNDERSCORE) {
            // Default case
            eat(parser, TOKEN_UNDERSCORE);
            pattern = create_identifier_node("_"); // Use identifier for underscore pattern
        } else {
            // Value pattern
            pattern = parse_expression(parser);
        }
        
        eat(parser, TOKEN_DOUBLE_ARROW); // =>
        
        // Parse body (should be a block)
        ASTNode *body = parse_block(parser);
        
        ASTNode *match_case = create_match_case_node(pattern, body);
        add_case_to_match(match_node, match_case);
    }
    
    eat(parser, TOKEN_RBRACE);
    
    return match_node;
}

ASTNode *parse_break_statement(Parser *parser) {
    eat(parser, TOKEN_BREAK);
    eat(parser, TOKEN_SEMICOLON);
    return create_break_node();
}

ASTNode *parse_continue_statement(Parser *parser) {
    eat(parser, TOKEN_CONTINUE);
    eat(parser, TOKEN_SEMICOLON);
    return create_continue_node();
}

BinaryOperator token_to_binary_operator(TokenType token) {
    switch (token) {
        case TOKEN_PLUS: return OP_ADD;
        case TOKEN_MINUS: return OP_SUB;
        case TOKEN_MULTIPLY: return OP_MUL;
        case TOKEN_DIVIDE: return OP_DIV;
        case TOKEN_EQ: return OP_EQ;
        case TOKEN_NE: return OP_NE;
        case TOKEN_LT: return OP_LT;
        case TOKEN_GT: return OP_GT;
        case TOKEN_LE: return OP_LE;
        case TOKEN_GE: return OP_GE;
        default: return OP_ADD; // This shouldn't happen
    }
}

ASTNode *parse_expression(Parser *parser) {
    return parse_comparison_expression(parser);
}

ASTNode *parse_comparison_expression(Parser *parser) {
    ASTNode *left = parse_additive_expression(parser);
    
    while (parser->current_token.type == TOKEN_EQ || parser->current_token.type == TOKEN_NE ||
           parser->current_token.type == TOKEN_LT || parser->current_token.type == TOKEN_GT ||
           parser->current_token.type == TOKEN_LE || parser->current_token.type == TOKEN_GE) {
        TokenType op_token = parser->current_token.type;
        eat(parser, op_token);
        ASTNode *right = parse_additive_expression(parser);
        left = create_binary_op_node(token_to_binary_operator(op_token), left, right);
    }
    
    return left;
}

ASTNode *parse_additive_expression(Parser *parser) {
    ASTNode *left = parse_multiplicative_expression(parser);
    return parse_additive_continuation(parser, left);
}

ASTNode *parse_additive_continuation(Parser *parser, ASTNode *left) {
    while (parser->current_token.type == TOKEN_PLUS || parser->current_token.type == TOKEN_MINUS) {
        TokenType op_token = parser->current_token.type;
        eat(parser, op_token);
        ASTNode *right = parse_multiplicative_expression(parser);
        left = create_binary_op_node(token_to_binary_operator(op_token), left, right);
    }
    
    return left;
}

ASTNode *parse_multiplicative_expression(Parser *parser) {
    ASTNode *left = parse_primary(parser);
    
    while (parser->current_token.type == TOKEN_MULTIPLY || parser->current_token.type == TOKEN_DIVIDE) {
        TokenType op_token = parser->current_token.type;
        eat(parser, op_token);
        ASTNode *right = parse_primary(parser);
        left = create_binary_op_node(token_to_binary_operator(op_token), left, right);
    }
    
    return left;
}

ASTNode *parse_primary(Parser *parser) {
    if (parser->current_token.type == TOKEN_AMPERSAND) {
        // Address-of operator
        eat(parser, TOKEN_AMPERSAND);
        ASTNode *operand = parse_primary(parser);
        return create_unary_op_node(UNARY_ADDRESS_OF, operand);
    } else if (parser->current_token.type == TOKEN_MULTIPLY) {
        // Dereference operator
        eat(parser, TOKEN_MULTIPLY);
        ASTNode *operand = parse_primary(parser);
        return create_unary_op_node(UNARY_DEREFERENCE, operand);
    } else if (parser->current_token.type == TOKEN_LPAREN) {
        eat(parser, TOKEN_LPAREN);
        ASTNode *expr = parse_expression(parser);
        eat(parser, TOKEN_RPAREN);
        return expr;
    } else if (parser->current_token.type == TOKEN_STRING) {
        char *value = strdup(parser->current_token.value);
        eat(parser, TOKEN_STRING);
        return create_literal_node(value, "string");
    } else if (parser->current_token.type == TOKEN_NUMBER) {
        char *value = strdup(parser->current_token.value);
        eat(parser, TOKEN_NUMBER);
        return create_literal_node(value, "i32");
    } else if (parser->current_token.type == TOKEN_FLOAT) {
        char *value = strdup(parser->current_token.value);
        eat(parser, TOKEN_FLOAT);
        return create_literal_node(value, "f32");
    } else if (parser->current_token.type == TOKEN_TRUE) {
        eat(parser, TOKEN_TRUE);
        return create_literal_node("true", "bool");
    } else if (parser->current_token.type == TOKEN_FALSE) {
        eat(parser, TOKEN_FALSE);
        return create_literal_node("false", "bool");
    } else if (parser->current_token.type == TOKEN_NULL) {
        eat(parser, TOKEN_NULL);
        return create_literal_node("null", "void");
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        char *name = strdup(parser->current_token.value);
        eat(parser, TOKEN_IDENTIFIER);
        
        if (parser->current_token.type == TOKEN_DOT) {
            eat(parser, TOKEN_DOT);
            if (parser->current_token.type != TOKEN_IDENTIFIER) {
                parser_error(parser, "Expected field or method name after '.'");
            }
            char *field_or_method = strdup(parser->current_token.value);
            eat(parser, TOKEN_IDENTIFIER);
            
            // Check for std.* qualified names first
            if (strcmp(name, "std") == 0) {
                // Qualified name (e.g., "std.print")
                char *qualified_name = malloc(strlen(name) + strlen(field_or_method) + 2);
                sprintf(qualified_name, "%s.%s", name, field_or_method);
                free(name);
                free(field_or_method);
                
                // Check if this is a function call
                if (parser->current_token.type == TOKEN_LPAREN) {
                    ASTNode *call = parse_call(parser, qualified_name);
                    free(qualified_name);
                    return call;
                } else {
                    ASTNode *id = create_identifier_node(qualified_name);
                    free(qualified_name);
                    return id;
                }
            } else if (parser->current_token.type == TOKEN_LPAREN) {
                // This is a method call: object.method()
                ASTNode *object = create_identifier_node(name);
                ASTNode *method_call = create_method_call_node(object, field_or_method);
                
                eat(parser, TOKEN_LPAREN);
                while (parser->current_token.type != TOKEN_RPAREN) {
                    ASTNode *arg = parse_expression(parser);
                    add_arg_to_method_call(method_call, arg);
                    
                    if (parser->current_token.type == TOKEN_COMMA) {
                        eat(parser, TOKEN_COMMA);
                    } else if (parser->current_token.type != TOKEN_RPAREN) {
                        parser_error(parser, "Expected ',' or ')' in method call");
                    }
                }
                eat(parser, TOKEN_RPAREN);
                
                free(name);
                free(field_or_method);
                return method_call;
            } else {
                // Field access: object.field
                ASTNode *object = create_identifier_node(name);
                ASTNode *field_access = create_field_access_node(object, field_or_method);
                
                free(name);
                free(field_or_method);
                return field_access;
            }
        } else if (parser->current_token.type == TOKEN_LPAREN) {
            ASTNode *call = parse_call(parser, name);
            free(name);
            return call;
        } else if (parser->current_token.type == TOKEN_LBRACE) {
            // Struct literal: TypeName { field: value, ... }
            // Only treat as struct literal if identifier starts with capital letter
            if (name[0] >= 'A' && name[0] <= 'Z') {
                ASTNode *struct_literal = create_struct_literal_node(name);
                
                eat(parser, TOKEN_LBRACE);
                
                while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
                    if (parser->current_token.type == TOKEN_NEWLINE) {
                        eat(parser, TOKEN_NEWLINE);
                        continue;
                    }
                    
                    if (parser->current_token.type != TOKEN_IDENTIFIER) {
                        parser_error(parser, "Expected field name in struct literal");
                    }
                    
                    char *field_name = strdup(parser->current_token.value);
                    eat(parser, TOKEN_IDENTIFIER);
                    
                    eat(parser, TOKEN_COLON);
                    
                    ASTNode *field_value = parse_expression(parser);
                    add_field_to_struct_literal(struct_literal, field_name, field_value);
                    
                    free(field_name);
                    
                    if (parser->current_token.type == TOKEN_COMMA) {
                        eat(parser, TOKEN_COMMA);
                    } else if (parser->current_token.type != TOKEN_RBRACE) {
                        // Allow newlines or end of struct
                        if (parser->current_token.type != TOKEN_NEWLINE) {
                            parser_error(parser, "Expected ',' or '}' in struct literal");
                        }
                    }
                }
                
                eat(parser, TOKEN_RBRACE);
                
                free(name);
                return struct_literal;
            } else {
                // Not a struct literal, just return the identifier
                ASTNode *identifier = create_identifier_node(name);
                free(name);
                return identifier;
            }
        } else {
            ASTNode *id = create_identifier_node(name);
            free(name);
            return id;
        }
    } else {
        parser_error(parser, "Unexpected token in expression");
        return NULL;
    }
}

ASTNode *parse_call(Parser *parser, const char *name) {
    ASTNode *call = create_call_node(name);
    
    eat(parser, TOKEN_LPAREN);
    
    if (parser->current_token.type != TOKEN_RPAREN) {
        ASTNode *arg = parse_expression(parser);
        add_arg_to_call(call, arg);
        
        while (parser->current_token.type == TOKEN_COMMA) {
            eat(parser, TOKEN_COMMA);
            arg = parse_expression(parser);
            add_arg_to_call(call, arg);
        }
    }
    
    eat(parser, TOKEN_RPAREN);
    
    return call;
}
