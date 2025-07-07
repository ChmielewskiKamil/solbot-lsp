#include "lexer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the next character from the lexer's input string.
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
  // The end of the input string is marked by null terminator.
  if (*(lexer->position) == '\0') {
    lexer->ch = 0; // EOF
    lexer->width = 0;
  }

  lexer->position += lexer->width;
}

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

Token *token_new(TokenType type, const char *literal, size_t literal_len) {
  Token *token = malloc(sizeof(*token));
  if (token == NULL) {
    return NULL;
  }

  token->type = type;

  // TODO: Instead of allocating memory for each literal this could be a 'view'
  // into the original input string. This would require changing the Token
  // struct by tracking start and len.
  token->literal = malloc(literal_len + 1);
  if (token->literal == NULL) {
    free(token);
    return NULL;
  }

  strncpy(token->literal, literal, literal_len);
  token->literal[literal_len] = '\0';

  return token;
}

Token *lexer_next_token(Lexer *lexer) {
  switch (lexer->ch) {
  case '{':
    return token_new(TOKEN_LBRACE, "{", 1);

  default:
    break;
  }
  return NULL;
}
