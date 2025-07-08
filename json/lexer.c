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
    lexer->ch = *(lexer->position);
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
    lexer_read_char(lexer);
    lexer->position += lexer->width;
  }
}

// TODO: For lexing strings and numbers, we will most likely need the
// lexer_backup function.

Lexer *lexer_new(const char *input_buffer) {
  Lexer *lexer = NULL;

  lexer = malloc(sizeof(*lexer));
  if (lexer == NULL) {
    return NULL;
  }

  lexer->input = input_buffer;
  lexer->position = input_buffer;

  lexer_read_char(lexer);

  return lexer;
}

void lexer_free(Lexer *lexer) {
  assert(lexer != NULL);
  free(lexer);
}

Token *token_new(TokenType type, const char *literal_start,
                 size_t literal_length) {
  Token *token = malloc(sizeof(*token));
  if (token == NULL) {
    return NULL;
  }

  token->type = type;
  token->literal_start = literal_start;
  token->literal_length = literal_length;

  return token;
}

Token *lexer_next_token(Lexer *lexer) {
  Token *token = NULL;

  switch (lexer->ch) {
  case '{':
    token = token_new(TOKEN_LBRACE, lexer->position, lexer->width);
    break;

  case '}':
    token = token_new(TOKEN_RBRACE, lexer->position, lexer->width);
    break;

  case '\0':
    token = token_new(TOKEN_EOF, lexer->position, lexer->width);
    break;

  default:
    break;
  }

  lexer_advance(lexer); // Advance so that lexer is ready for the next call.

  return token;
}
