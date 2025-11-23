#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer.h"
#include "libs/foundation.h"

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
static void lexer_read_char(Lexer *lexer) {
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
static void lexer_advance(Lexer *lexer) {
  if (lexer->ch != '\0') {
    lexer->position += lexer->width;
    lexer_read_char(lexer);
  }
}

static void lexer_skip_whitespace(Lexer *lexer) {
  while (lexer->ch == ' ' || lexer->ch == '\n' || lexer->ch == '\t' ||
         lexer->ch == '\r') {
    lexer_advance(lexer);
  }
}

static inline bool is_digit(uint32_t ch) {
  if (ch >= '0' && ch <= '9') {
    return true;
  }

  return false;
}

Lexer lexer_new(const char *input_buffer) {
  Lexer lexer;

  lexer.input = input_buffer;
  lexer.position = input_buffer;

  lexer_read_char(&lexer);

  return lexer;
}

Token lex_string(Lexer *lexer) {
  Token tkn;
  tkn.type = TOKEN_STRING;
  tkn.literal_start = lexer->position;

  lexer_advance(lexer); // Move past the opening quotation mark.

  while (lexer->ch != '"' && lexer->ch != '\0') {
    // We encountered an escaped character e.g. \", \n or \0 (there are others
    // too). Most of those we can skip, only \0 requires special treatment.
    if (lexer->ch == '\\') {
      lexer_advance(lexer); // Move past the backslash.

      if (lexer->ch != '\0') {
        lexer_advance(lexer); // Move past the char e.g. n, t, r.
      }

      continue;
    }

    lexer_advance(lexer); // Consume string content.
  }

  if (lexer->ch == '"') {
    lexer_advance(lexer);
    tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
  }

  return tkn;
}

Token lex_number(Lexer *lexer) {
  Token tkn;
  tkn.type = TOKEN_NUMBER;
  tkn.literal_start = lexer->position;
  tkn.literal_length = 0;

  // Consume an optional minus sign.
  if (lexer->ch == '-') {
    lexer_advance(lexer);
  }

  // Unsigned integer in JSON shouldn't start with `0`. If the number starts
  // with `0` it means we are either dealing with a float (fraction) or an
  // exponential. Otherwise it is an invalid number.
  if (lexer->ch == '0') {
    lexer_advance(lexer); // Consume `0` digit.

    // Digits after `0` are not allowed e.g. `09` is invalid.
    if (is_digit(lexer->ch)) {
      tkn.type = TOKEN_ILLEGAL;
      tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
      return tkn;
    }
  } else {
    // The only way to land in this case is when there was a minus sign before.
    // The minus sign has to be followed by a digit. Otherwise the number is
    // invalid.
    if (!is_digit(lexer->ch)) {
      tkn.type = TOKEN_ILLEGAL;
      tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
      return tkn;
    }

    while (is_digit(lexer->ch)) {
      lexer_advance(lexer);
    }
  }

  // We are dealing with a fraction (floating point number).
  if (lexer->ch == '.') {
    lexer_advance(lexer); // Consume the `.` (dot) char.

    // After the dot there has to be at least one digit.
    if (!is_digit(lexer->ch)) {
      tkn.type = TOKEN_ILLEGAL;
      tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
      return tkn;
    }

    while (is_digit(lexer->ch)) {
      lexer_advance(lexer);
    }
  }

  // The number has an exponent part.
  if (lexer->ch == 'e' || lexer->ch == 'E') {
    lexer_advance(lexer);

    // After the exponent 'e' or 'E' char there can be an optional '+' or '-'
    // sign.
    if (lexer->ch == '-' || lexer->ch == '+') {
      lexer_advance(lexer);
    }

    // The only thing that can be present after the exponent char (and/or the
    // sign) is a digit.
    if (!is_digit(lexer->ch)) {
      tkn.type = TOKEN_ILLEGAL;
      tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
      return tkn;
    }

    while (is_digit(lexer->ch)) {
      lexer_advance(lexer);
    }
  }

  tkn.literal_length = (size_t)(lexer->position - tkn.literal_start);
  return tkn;
}

Token lexer_next_token(Lexer *lexer) {
  lexer_skip_whitespace(lexer);

  Token token;
  // Default initialization
  token.literal_start = lexer->position;
  token.literal_length = 1;

  // IMPORTANT: The pattern followed in the lexer_next_token function is that
  // each case in the switch statement advances the lexer position in such a way
  // that at the end of execution, the lexer is sitting on the next thing to be
  // processed.

  switch (lexer->ch) {
  case '{':
    token.type = TOKEN_LBRACE;
    lexer_advance(lexer);
    break;

  case '}':
    token.type = TOKEN_RBRACE;
    lexer_advance(lexer);
    break;

  case ',':
    token.type = TOKEN_COMMA;
    lexer_advance(lexer);
    break;

  case ':':
    token.type = TOKEN_COLON;
    lexer_advance(lexer);
    break;

  case '"':
    return lex_string(lexer);

  case '\0':
    token.type = TOKEN_EOF;
    token.literal_length = 0;
    break;

  default:

    if (is_digit((lexer->ch)) || lexer->ch == '-') {
      return lex_number(lexer);
    }

    token.type = TOKEN_ILLEGAL;
    token.literal_length = lexer->width; // handles illegal multi byte chars if
                                         // UTF8 support is ever added.
    lexer_advance(lexer);
    break;
  }

  return token;
}
