#include "lexer.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct Lexer {
  const char *input;    // The JSON string being lexed.
  const char *position; // The current position in the JSON string.
  uint32_t ch;          // The current character (4 byte code point).
  uint8_t width;        // The width of the last read character in bytes.
};

/**
 * Reads the next character from the lexer's input string. This function DOES
 * NOT advance the lexer position. It just reads the char that the lexer
 * currently points to.
 *
 * NOTE (for future expansion): This is a simplified, ASCII-only implementation.
 * It assumes every character is one byte. To support full Unicode, this
 * function is the only place that needs to be upgraded with UTF-8 decoding
 * logic. The rest of the lexer can remain the same because it operates on the
 * 32-bit `ch`.
 *
 * @param lexer A pointer to the Lexer whose state will be updated.
 */
void lexer_read_char(Lexer *lexer) {
  // The end of the input string is marked by the null terminator.
  if (*(lexer->position) == '\0') {
    lexer->ch = '\0';
    lexer->width = 0;
  } else {
    lexer->ch = (unsigned char)*(lexer->position);
    lexer->width = 1; // TODO: Implement proper UTF-8 decoding.
  }
}

/**
 * Advances the lexer's position to the next character.
 *
 * @param lexer A pointer to the Lexer whose state will be advanced.
 */
void lexer_advance(Lexer *lexer) {
  if (lexer->ch != '\0') {
    lexer->position += lexer->width;
    lexer_read_char(lexer);
  }
}

// TODO: For lexing strings and numbers, we will most likely need the
// lexer_backup function.

Lexer lexer_new(const char *input_buffer) {
  Lexer lexer;

  lexer.input = input_buffer;
  lexer.position = input_buffer;

  lexer_read_char(&lexer);

  return lexer;
}

Token token_new(Lexer *lexer, TokenType type) {
  Token token;

  token.type = type;
  token.literal_start = lexer->position;
  token.literal_length = lexer->width;

  return token;
}

Token lexer_next_token(Lexer *lexer) {
  Token token;

  switch (lexer->ch) {
  case '{':
    token = token_new(lexer, TOKEN_LBRACE);
    break;

  case '}':
    token = token_new(lexer, TOKEN_RBRACE);
    break;

  case '\0':
    token = token_new(lexer, TOKEN_EOF);
    break;

  default:
    token = token_new(lexer, TOKEN_ILLEGAL);
    break;
  }

  // Advance so that lexer is ready for the next call. The EOF case is handled
  // explicitly by the advance function so it does not need special treatment
  // here.
  lexer_advance(lexer);

  return token;
}
