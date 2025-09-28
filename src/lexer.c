#define _GNU_SOURCE
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Lexer *create_lexer(const char *input) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->current_char = input[0];
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void free_lexer(Lexer *lexer) {
    free(lexer);
}

void advance(Lexer *lexer) {
    lexer->position++;
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    if (lexer->position >= strlen(lexer->input)) {
        lexer->current_char = '\0';
    } else {
        lexer->current_char = lexer->input[lexer->position];
    }
}

char peek(Lexer *lexer) {
    int peek_pos = lexer->position + 1;
    if (peek_pos >= strlen(lexer->input)) {
        return '\0';
    }
    return lexer->input[peek_pos];
}

void skip_whitespace(Lexer *lexer) {
    while (lexer->current_char != '\0' && 
           (lexer->current_char == ' ' || lexer->current_char == '\t' || lexer->current_char == '\r')) {
        advance(lexer);
    }
}

char *read_string(Lexer *lexer) {
    advance(lexer); // skip opening quote
    int start = lexer->position;
    
    while (lexer->current_char != '\0' && lexer->current_char != '"') {
        advance(lexer);
    }
    
    int length = lexer->position - start;
    char *string = malloc(length + 1);
    strncpy(string, &lexer->input[start], length);
    string[length] = '\0';
    
    if (lexer->current_char == '"') {
        advance(lexer); // skip closing quote
    }
    
    return string;
}

void skip_comment(Lexer *lexer) {
    // Skip '//' comment until end of line
    if (lexer->current_char == '/' && peek(lexer) == '/') {
        advance(lexer); // skip first '/'
        advance(lexer); // skip second '/'
        
        while (lexer->current_char != '\0' && lexer->current_char != '\n') {
            advance(lexer);
        }
    }
}

char *read_float(Lexer *lexer) {
    int start = lexer->position;
    
    // Read integer part
    while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
        advance(lexer);
    }
    
    // Check for decimal point
    if (lexer->current_char == '.') {
        advance(lexer); // consume '.'
        
        // Read fractional part
        while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
            advance(lexer);
        }
    }
    
    int length = lexer->position - start;
    char *float_str = malloc(length + 1);
    strncpy(float_str, &lexer->input[start], length);
    float_str[length] = '\0';
    
    return float_str;
}

char *read_number(Lexer *lexer) {
    int start = lexer->position;
    
    while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
        advance(lexer);
    }
    
    int length = lexer->position - start;
    char *number = malloc(length + 1);
    strncpy(number, &lexer->input[start], length);
    number[length] = '\0';
    
    return number;
}

char *read_identifier(Lexer *lexer) {
    int start = lexer->position;
    
    while (lexer->current_char != '\0' && 
           (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        advance(lexer);
    }
    
    int length = lexer->position - start;
    char *identifier = malloc(length + 1);
    strncpy(identifier, &lexer->input[start], length);
    identifier[length] = '\0';
    
    return identifier;
}

TokenType get_keyword_type(const char *identifier) {
    if (strcmp(identifier, "import") == 0) return TOKEN_IMPORT;
    if (strcmp(identifier, "fn") == 0) return TOKEN_FN;
    if (strcmp(identifier, "def") == 0) return TOKEN_DEF;
    if (strcmp(identifier, "mut") == 0) return TOKEN_MUT;
    if (strcmp(identifier, "const") == 0) return TOKEN_CONST;
    if (strcmp(identifier, "return") == 0) return TOKEN_RETURN;
    if (strcmp(identifier, "bool") == 0) return TOKEN_BOOL;
    if (strcmp(identifier, "i32") == 0) return TOKEN_I32;
    if (strcmp(identifier, "f32") == 0) return TOKEN_F32;
    if (strcmp(identifier, "string") == 0) return TOKEN_STRING_TYPE;
    if (strcmp(identifier, "void") == 0) return TOKEN_VOID;
    if (strcmp(identifier, "true") == 0) return TOKEN_TRUE;
    if (strcmp(identifier, "false") == 0) return TOKEN_FALSE;
    if (strcmp(identifier, "null") == 0) return TOKEN_NULL;
    if (strcmp(identifier, "struct") == 0) return TOKEN_STRUCT;
    if (strcmp(identifier, "enum") == 0) return TOKEN_ENUM;
    if (strcmp(identifier, "pub") == 0) return TOKEN_PUB;
    if (strcmp(identifier, "priv") == 0) return TOKEN_PRIV;
    if (strcmp(identifier, "static") == 0) return TOKEN_STATIC;
    if (strcmp(identifier, "self") == 0) return TOKEN_SELF;
    if (strcmp(identifier, "if") == 0) return TOKEN_IF;
    if (strcmp(identifier, "unless") == 0) return TOKEN_UNLESS;
    if (strcmp(identifier, "else") == 0) return TOKEN_ELSE;
    if (strcmp(identifier, "for") == 0) return TOKEN_FOR;
    if (strcmp(identifier, "while") == 0) return TOKEN_WHILE;
    if (strcmp(identifier, "switch") == 0) return TOKEN_SWITCH;
    if (strcmp(identifier, "match") == 0) return TOKEN_MATCH;
    if (strcmp(identifier, "case") == 0) return TOKEN_CASE;
    if (strcmp(identifier, "default") == 0) return TOKEN_DEFAULT;
    if (strcmp(identifier, "break") == 0) return TOKEN_BREAK;
    if (strcmp(identifier, "continue") == 0) return TOKEN_CONTINUE;
    if (strcmp(identifier, "defer") == 0) return TOKEN_DEFER;
    if (strcmp(identifier, "deferred") == 0) return TOKEN_DEFERRED;
    if (strcmp(identifier, "spawnable") == 0) return TOKEN_SPAWNABLE;
    if (strcmp(identifier, "run") == 0) return TOKEN_RUN;
    return TOKEN_IDENTIFIER;
}

Token next_token(Lexer *lexer) {
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;
    
    while (lexer->current_char != '\0') {
        if (lexer->current_char == ' ' || lexer->current_char == '\t' || lexer->current_char == '\r') {
            skip_whitespace(lexer);
            continue;
        }
        
        if (lexer->current_char == '\n') {
            token.type = TOKEN_NEWLINE;
            token.value = strdup("\n");
            advance(lexer);
            return token;
        }
        
        // Handle comments
        if (lexer->current_char == '/' && peek(lexer) == '/') {
            skip_comment(lexer);
            continue;
        }
        
        if (lexer->current_char == '"') {
            token.type = TOKEN_STRING;
            token.value = read_string(lexer);
            return token;
        }
        
        if (isdigit(lexer->current_char)) {
            // Check if it's a float (contains decimal point)
            int temp_pos = lexer->position;
            int has_decimal = 0;
            
            // Look ahead to see if there's a decimal point
            while (temp_pos < strlen(lexer->input) && 
                   (isdigit(lexer->input[temp_pos]) || lexer->input[temp_pos] == '.')) {
                if (lexer->input[temp_pos] == '.') {
                    has_decimal = 1;
                    break;
                }
                temp_pos++;
            }
            
            if (has_decimal) {
                token.type = TOKEN_FLOAT;
                token.value = read_float(lexer);
            } else {
                token.type = TOKEN_NUMBER;
                token.value = read_number(lexer);
            }
            return token;
        }
        
        if (isalpha(lexer->current_char) || lexer->current_char == '_') {
            char *identifier = read_identifier(lexer);
            token.type = get_keyword_type(identifier);
            token.value = identifier;
            return token;
        }
        
        switch (lexer->current_char) {
            case '(':
                token.type = TOKEN_LPAREN;
                token.value = strdup("(");
                advance(lexer);
                return token;
            case ')':
                token.type = TOKEN_RPAREN;
                token.value = strdup(")");
                advance(lexer);
                return token;
            case '{':
                token.type = TOKEN_LBRACE;
                token.value = strdup("{");
                advance(lexer);
                return token;
            case '}':
                token.type = TOKEN_RBRACE;
                token.value = strdup("}");
                advance(lexer);
                return token;
            case ';':
                token.type = TOKEN_SEMICOLON;
                token.value = strdup(";");
                advance(lexer);
                return token;
            case ':':
                if (peek(lexer) == ':') {
                    token.type = TOKEN_DOUBLE_COLON;
                    token.value = strdup("::");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    token.type = TOKEN_COLON;
                    token.value = strdup(":");
                    advance(lexer);
                    return token;
                }
            case '=':
                if (peek(lexer) == '=') {
                    token.type = TOKEN_EQ;
                    token.value = strdup("==");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else if (peek(lexer) == '>') {
                    token.type = TOKEN_DOUBLE_ARROW;
                    token.value = strdup("=>");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    token.type = TOKEN_ASSIGN;
                    token.value = strdup("=");
                    advance(lexer);
                    return token;
                }
            case '+':
                token.type = TOKEN_PLUS;
                token.value = strdup("+");
                advance(lexer);
                return token;
            case '-':
                if (peek(lexer) == '>') {
                    token.type = TOKEN_ARROW;
                    token.value = strdup("->");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    token.type = TOKEN_MINUS;
                    token.value = strdup("-");
                    advance(lexer);
                    return token;
                }
            case '*':
                token.type = TOKEN_MULTIPLY;
                token.value = strdup("*");
                advance(lexer);
                return token;
            case '/':
                token.type = TOKEN_DIVIDE;
                token.value = strdup("/");
                advance(lexer);
                return token;
            case '!':
                if (peek(lexer) == '=') {
                    token.type = TOKEN_NE;
                    token.value = strdup("!=");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    // '!' by itself is not supported yet
                    token.type = TOKEN_UNKNOWN;
                    token.value = strdup("!");
                    advance(lexer);
                    return token;
                }
            case '<':
                if (peek(lexer) == '=') {
                    token.type = TOKEN_LE;
                    token.value = strdup("<=");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    token.type = TOKEN_LT;
                    token.value = strdup("<");
                    advance(lexer);
                    return token;
                }
            case '>':
                if (peek(lexer) == '=') {
                    token.type = TOKEN_GE;
                    token.value = strdup(">=");
                    advance(lexer);
                    advance(lexer);
                    return token;
                } else {
                    token.type = TOKEN_GT;
                    token.value = strdup(">");
                    advance(lexer);
                    return token;
                }
            case '&':
                token.type = TOKEN_AMPERSAND;
                token.value = strdup("&");
                advance(lexer);
                return token;
            case '.':
                token.type = TOKEN_DOT;
                token.value = strdup(".");
                advance(lexer);
                return token;
            case '@':
                token.type = TOKEN_AT;
                token.value = strdup("@");
                advance(lexer);
                return token;
            case '#':
                token.type = TOKEN_HASH;
                token.value = strdup("#");
                advance(lexer);
                return token;
            case ',':
                token.type = TOKEN_COMMA;
                token.value = strdup(",");
                advance(lexer);
                return token;
            case '_':
                token.type = TOKEN_UNDERSCORE;
                token.value = strdup("_");
                advance(lexer);
                return token;
        }
        
        // Unknown character
        token.type = TOKEN_UNKNOWN;
        char *unknown = malloc(2);
        unknown[0] = lexer->current_char;
        unknown[1] = '\0';
        token.value = unknown;
        advance(lexer);
        return token;
    }
    
    token.type = TOKEN_EOF;
    token.value = strdup("");
    return token;
}

void free_token(Token *token) {
    if (token->value) {
        free(token->value);
    }
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_FN: return "FN";
        case TOKEN_DEF: return "DEF";
        case TOKEN_MUT: return "MUT";
        case TOKEN_CONST: return "CONST";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_I32: return "I32";
        case TOKEN_F32: return "F32";
        case TOKEN_STRING_TYPE: return "STRING_TYPE";
        case TOKEN_VOID: return "VOID";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_NULL: return "NULL";
        case TOKEN_STRUCT: return "STRUCT";
        case TOKEN_ENUM: return "ENUM";
        case TOKEN_PUB: return "PUB";
        case TOKEN_PRIV: return "PRIV";
        case TOKEN_STATIC: return "STATIC";
        case TOKEN_SELF: return "SELF";
        case TOKEN_IF: return "IF";
        case TOKEN_UNLESS: return "UNLESS";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_SWITCH: return "SWITCH";
        case TOKEN_MATCH: return "MATCH";
        case TOKEN_CASE: return "CASE";
        case TOKEN_DEFAULT: return "DEFAULT";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_DEFER: return "DEFER";
        case TOKEN_DEFERRED: return "DEFERRED";
        case TOKEN_SPAWNABLE: return "SPAWNABLE";
        case TOKEN_RUN: return "RUN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_DOUBLE_COLON: return "DOUBLE_COLON";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_DOUBLE_ARROW: return "DOUBLE_ARROW";
        case TOKEN_DOT: return "DOT";
        case TOKEN_AT: return "AT";
        case TOKEN_HASH: return "HASH";
        case TOKEN_QUOTE: return "QUOTE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NE: return "NE";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LE: return "LE";
        case TOKEN_GE: return "GE";
        case TOKEN_AMPERSAND: return "AMPERSAND";
        case TOKEN_UNDERSCORE: return "UNDERSCORE";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}
