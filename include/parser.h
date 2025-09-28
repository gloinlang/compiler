#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current_token;
} Parser;

// Parser functions
Parser *create_parser(Lexer *lexer);
void free_parser(Parser *parser);
void eat(Parser *parser, TokenType expected_type);
void parser_error(Parser *parser, const char *message);

// Parsing functions
ASTNode *parse_program(Parser *parser);
ASTNode *parse_file(const char *filename);
ASTNode *parse_import(Parser *parser);
ASTNode *parse_function_declaration(Parser *parser);
ASTNode *parse_struct_declaration(Parser *parser);
ASTNode *parse_enum_declaration(Parser *parser);
ASTNode *parse_struct(Parser *parser);
ASTNode *parse_block(Parser *parser);
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_variable_declaration(Parser *parser);
ASTNode *parse_assignment_statement(Parser *parser);
ASTNode *parse_return_statement(Parser *parser);
ASTNode *parse_expression_statement(Parser *parser);
ASTNode *parse_if_statement(Parser *parser);
ASTNode *parse_unless_statement(Parser *parser);
ASTNode *parse_for_statement(Parser *parser);
ASTNode *parse_while_statement(Parser *parser);
ASTNode *parse_switch_statement(Parser *parser);
ASTNode *parse_match_statement(Parser *parser);
ASTNode *parse_break_statement(Parser *parser);
ASTNode *parse_continue_statement(Parser *parser);
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_comparison_expression(Parser *parser);
ASTNode *parse_additive_expression(Parser *parser);
ASTNode *parse_additive_continuation(Parser *parser, ASTNode *left);
ASTNode *parse_multiplicative_expression(Parser *parser);
ASTNode *parse_call(Parser *parser, const char *name);
ASTNode *parse_primary(Parser *parser);
ImportType parse_import_type(Parser *parser);
BinaryOperator token_to_binary_operator(TokenType token);

#endif
