#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IMPORT,
    TOKEN_FN,
    TOKEN_DEF,
    TOKEN_MUT,
    TOKEN_CONST,      // NEW: const keyword
    TOKEN_RETURN,
    TOKEN_BOOL,
    TOKEN_I32,        // NEW: explicit i32 type
    TOKEN_F32,        // NEW: f32 type  
    TOKEN_STRING_TYPE, // NEW: explicit string type
    TOKEN_VOID,       // NEW: void type
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,       // NEW: null literal
    TOKEN_STRUCT,
    TOKEN_ENUM,       // NEW: enum keyword
    TOKEN_PUB,
    TOKEN_PRIV,
    TOKEN_STATIC,     // NEW: static keyword
    TOKEN_SELF,       // NEW: self keyword
    TOKEN_IF,
    TOKEN_UNLESS,
    TOKEN_ELSE,       // Already exists but now officially supported
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_SWITCH,
    TOKEN_MATCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_DEFER,      // NEW: defer keyword
    TOKEN_DEFERRED,   // NEW: deferred keyword
    TOKEN_SPAWNABLE,  // NEW: spawnable keyword
    TOKEN_RUN,        // NEW: run keyword
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_FLOAT,      // NEW: floating point numbers
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON, // NEW: :: operator
    TOKEN_ASSIGN,
    TOKEN_ARROW,
    TOKEN_DOUBLE_ARROW, // =>
    TOKEN_DOT,
    TOKEN_AT,      // @
    TOKEN_HASH,    // #
    TOKEN_QUOTE,   // "
    TOKEN_COMMA,
    TOKEN_PLUS,    // +
    TOKEN_MINUS,   // -
    TOKEN_MULTIPLY, // *
    TOKEN_DIVIDE,  // /
    TOKEN_COMMENT, // NEW: // comments
    TOKEN_EQ,      // ==
    TOKEN_NE,      // !=
    TOKEN_LT,      // <
    TOKEN_GT,      // >
    TOKEN_LE,      // <=
    TOKEN_GE,      // >=
    TOKEN_AMPERSAND, // & (address-of)
    TOKEN_UNDERSCORE, // _
    TOKEN_NEWLINE,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct {
    const char *input;
    int position;
    int current_char;
    int line;
    int column;
} Lexer;

// Lexer functions
Lexer *create_lexer(const char *input);
void free_lexer(Lexer *lexer);
Token next_token(Lexer *lexer);
void skip_whitespace(Lexer *lexer);
void skip_comment(Lexer *lexer);  // NEW: Skip single-line comments
void advance(Lexer *lexer);
char peek(Lexer *lexer);
char *read_string(Lexer *lexer);
char *read_number(Lexer *lexer);
char *read_float(Lexer *lexer);   // NEW: Read floating point numbers
char *read_identifier(Lexer *lexer);
TokenType get_keyword_type(const char *identifier);

// Token functions
void free_token(Token *token);
const char *token_type_to_string(TokenType type);

#endif
