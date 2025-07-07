#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include <stdint.h>
typedef enum {
  TOKEN_ILLEGAL, // unknown/illegal character
  TOKEN_EOF,     // end of the input string

  // Punctuators/Delimeters
  TOKEN_LBRACE,   // {
  TOKEN_RBRACE,   // }
  TOKEN_LBRACKET, // [
  TOKEN_RBRACKET, // ]
  TOKEN_COMMA,    // ,
  TOKEN_COLON,    // :

  // Literals
  TOKEN_STRING, // "hello world", "method" etc.
  TOKEN_NUMBER, // 123, TODO: Does LSP utilize floating point or scientific?
  TOKEN_TRUE,   // true
  TOKEN_FALSE,  // false
  TOKEN_NULL,   // null
} TokenType;

typedef struct {
  TokenType type;
  char *literal;
} Token;

typedef struct {
  const char *input;    // The JSON string being lexed.
  const char *position; // The current position in the JSON string.
  uint32_t ch;          // The current character (4 byte code point).
  uint8_t width;        // The width of the last read character in bytes.
} Lexer;

Lexer *lexer_new(const char *input_buffer);
Token *lexer_next_token(Lexer *lexer);

#endif // JSON_LEXER_H
