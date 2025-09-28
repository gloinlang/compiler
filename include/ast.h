#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include "types.h"

typedef enum {
    NODE_PROGRAM,
    NODE_IMPORT,
    NODE_FUNCTION,
    NODE_PARAMETER,
    NODE_VARIABLE_DECL,  // Used for def, def mut, and def const
    NODE_ASSIGNMENT,
    NODE_POINTER_ASSIGNMENT,
    NODE_RETURN,
    NODE_CALL,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_BLOCK,
    NODE_STRUCT,
    NODE_STRUCT_FIELD,
    NODE_STRUCT_METHOD,
    NODE_FIELD_ACCESS,
    NODE_METHOD_CALL,
    NODE_STRUCT_LITERAL,
    NODE_ENUM,           // enum declarations
    NODE_ENUM_VARIANT,   // enum variants
    NODE_IF,
    NODE_UNLESS,
    NODE_FOR,
    NODE_WHILE,
    NODE_SWITCH,
    NODE_SWITCH_CASE,
    NODE_MATCH,
    NODE_MATCH_CASE,
    NODE_BREAK,
    NODE_CONTINUE
} NodeType;

typedef enum {
    IMPORT_STD,      // @std
    IMPORT_EXTERNAL, // #lib
    IMPORT_LOCAL     // ./lib
} ImportType;

typedef enum {
    OP_ADD,     // +
    OP_SUB,     // -
    OP_MUL,     // *
    OP_DIV,     // /
    OP_EQ,      // ==
    OP_NE,      // !=
    OP_LT,      // <
    OP_GT,      // >
    OP_LE,      // <=
    OP_GE       // >=
} BinaryOperator;

typedef enum {
    UNARY_ADDRESS_OF,  // &
    UNARY_DEREFERENCE  // *
} UnaryOperator;

typedef enum {
    VISIBILITY_PUBLIC,   // pub
    VISIBILITY_PRIVATE   // priv
} Visibility;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            struct ASTNode **imports;
            struct ASTNode **functions;
            int import_count;
            int function_count;
        } program;
        
        struct {
            ImportType import_type;
            char *path;
        } import;
        
        struct {
            char *name;
            char *return_type;
            struct ASTNode **params;
            struct ASTNode *body;
            int param_count;
        } function;
        
        struct {
            char *name;
            char *type;
            TypeKind resolved_type;  // Parsed type
        } parameter;
        
        struct {
            char *name;
            char *type;
            struct ASTNode *value;
            int is_mutable;  // -1 = const, 0 = immutable, 1 = mutable
            TypeKind resolved_type;  // Parsed type
        } variable_decl;
        
        struct {
            char *variable_name;
            struct ASTNode *value;
        } assignment;
        
        struct {
            struct ASTNode *target;  // Left-hand side expression (e.g., dereference)
            struct ASTNode *value;   // Right-hand side value
        } pointer_assignment;
        
        struct {
            struct ASTNode *value;
        } return_stmt;
        
        struct {
            char *name;
            struct ASTNode **args;
            int arg_count;
        } call;
        
        struct {
            char *name;
            TypeKind resolved_type;  // Type determined during analysis
        } identifier;
        
        struct {
            char *value;
            char *type; // "string", "i32", etc.
            TypeKind resolved_type;  // Parsed type
        } literal;
        
        struct {
            BinaryOperator operator;
            struct ASTNode *left;
            struct ASTNode *right;
            TypeKind resolved_type;  // Result type of operation
        } binary_op;
        
        struct {
            UnaryOperator operator;
            struct ASTNode *operand;
            TypeKind resolved_type;  // Result type of operation
        } unary_op;
        
        struct {
            struct ASTNode **statements;
            int statement_count;
        } block;
        
        struct {
            char *name;
            struct ASTNode **fields;
            struct ASTNode **methods;
            int field_count;
            int method_count;
        } struct_decl;
        
        struct {
            char *name;
            char *type;
            TypeKind resolved_type;
        } struct_field;
        
        struct {
            char *name;
            char *return_type;
            struct ASTNode **params;
            struct ASTNode *body;
            int param_count;
            Visibility visibility;
        } struct_method;
        
        struct {
            struct ASTNode *object;  // The object being accessed
            char *field_name;        // Name of the field
            TypeKind resolved_type;  // Type of the field
        } field_access;
        
        struct {
            struct ASTNode *object;  // The object on which method is called
            char *method_name;       // Name of the method
            struct ASTNode **args;   // Method arguments
            int arg_count;
            TypeKind resolved_type;  // Return type of the method
        } method_call;
        
        struct {
            char *struct_type_name;  // Name of the struct type
            struct ASTNode **field_values;  // Field initialization values
            char **field_names;      // Names of fields being initialized
            int field_count;
            TypeKind resolved_type;  // The struct type
        } struct_literal;
        
        struct {
            char *name;              // Name of the enum
            struct ASTNode **variants; // Enum variants
            int variant_count;
        } enum_decl;
        
        struct {
            char *name;              // Name of the variant
        } enum_variant;
        
        struct {
            struct ASTNode *condition;  // Boolean condition
            struct ASTNode *then_block; // Block to execute if condition is true
            struct ASTNode *else_block; // Optional else block (can be NULL)
        } if_stmt;
        
        struct {
            struct ASTNode *condition;  // Boolean condition
            struct ASTNode *then_block; // Block to execute if condition is false
            struct ASTNode *else_block; // Optional else block (can be NULL)
        } unless_stmt;
        
        struct {
            struct ASTNode *init;       // Optional initialization statement
            struct ASTNode *condition;  // Loop condition
            struct ASTNode *update;     // Optional update statement
            struct ASTNode *body;       // Loop body
        } for_stmt;
        
        struct {
            struct ASTNode *condition;  // Loop condition
            struct ASTNode *body;       // Loop body
        } while_stmt;
        
        struct {
            struct ASTNode *expression;     // Expression to switch on
            struct ASTNode **cases;         // Array of switch cases
            int case_count;
            struct ASTNode *default_case;   // Optional default case
        } switch_stmt;
        
        struct {
            struct ASTNode *value;          // Case value to match
            struct ASTNode **statements;    // Statements to execute
            int statement_count;
        } switch_case;
        
        struct {
            struct ASTNode *expression;     // Expression to match on
            struct ASTNode **cases;         // Array of match cases
            int case_count;
        } match_stmt;
        
        struct {
            struct ASTNode *pattern;        // Pattern to match (value or _)
            struct ASTNode *body;           // Block to execute
        } match_case;
    } data;
} ASTNode;

// AST creation functions
ASTNode *create_program_node(void);
ASTNode *create_import_node(ImportType type, const char *path);
ASTNode *create_function_node(const char *name, const char *return_type);
ASTNode *create_parameter_node(const char *name, const char *type);
ASTNode *create_variable_decl_node(const char *name, const char *type, ASTNode *value, int is_mutable);
ASTNode *create_assignment_node(const char *variable_name, ASTNode *value);
ASTNode *create_pointer_assignment_node(ASTNode *target, ASTNode *value);
ASTNode *create_return_node(ASTNode *value);
ASTNode *create_call_node(const char *name);
ASTNode *create_identifier_node(const char *name);
ASTNode *create_literal_node(const char *value, const char *type);
ASTNode *create_binary_op_node(BinaryOperator operator, ASTNode *left, ASTNode *right);
ASTNode *create_unary_op_node(UnaryOperator operator, ASTNode *operand);
ASTNode *create_block_node(void);
ASTNode *create_struct_node(const char *name);
ASTNode *create_struct_field_node(const char *name, const char *type);
ASTNode *create_struct_method_node(const char *name, const char *return_type, Visibility visibility);
ASTNode *create_field_access_node(ASTNode *object, const char *field_name);
ASTNode *create_method_call_node(ASTNode *object, const char *method_name);
ASTNode *create_struct_literal_node(const char *struct_type_name);
ASTNode *create_enum_node(const char *name);
ASTNode *create_enum_variant_node(const char *name);
ASTNode *create_if_node(ASTNode *condition, ASTNode *then_block, ASTNode *else_block);
ASTNode *create_unless_node(ASTNode *condition, ASTNode *then_block, ASTNode *else_block);
ASTNode *create_for_node(ASTNode *init, ASTNode *condition, ASTNode *update, ASTNode *body);
ASTNode *create_while_node(ASTNode *condition, ASTNode *body);
ASTNode *create_switch_node(ASTNode *expression);
ASTNode *create_switch_case_node(ASTNode *value);
ASTNode *create_match_node(ASTNode *expression);
ASTNode *create_match_case_node(ASTNode *pattern, ASTNode *body);
ASTNode *create_break_node(void);
ASTNode *create_continue_node(void);

// AST utility functions
void add_import_to_program(ASTNode *program, ASTNode *import);
void add_function_to_program(ASTNode *program, ASTNode *function);
void add_parameter_to_function(ASTNode *function, ASTNode *parameter);
void add_statement_to_block(ASTNode *block, ASTNode *statement);
void add_arg_to_call(ASTNode *call, ASTNode *arg);
void add_field_to_struct(ASTNode *struct_node, ASTNode *field);
void add_method_to_struct(ASTNode *struct_node, ASTNode *method);
void add_parameter_to_struct_method(ASTNode *method, ASTNode *parameter);
void add_arg_to_method_call(ASTNode *method_call, ASTNode *arg);
void add_field_to_struct_literal(ASTNode *struct_literal, const char *field_name, ASTNode *value);
void add_enum_variant(ASTNode *enum_node, ASTNode *variant);
void add_case_to_switch(ASTNode *switch_stmt, ASTNode *switch_case);
void add_statement_to_switch_case(ASTNode *switch_case, ASTNode *statement);
void add_case_to_match(ASTNode *match_stmt, ASTNode *match_case);
void set_switch_default(ASTNode *switch_stmt, ASTNode *default_case);
void free_ast_node(ASTNode *node);

// Type analysis functions  
void resolve_types(ASTNode *node);
TypeKind get_node_type(ASTNode *node);

#endif
