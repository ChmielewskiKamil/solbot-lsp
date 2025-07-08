#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include <stddef.h>
#include <stdint.h>

//////////// TOKENS /////////////

/**
 * @enum TokenType
 * @brief Defines the different types of tokens the lexer can recognize.
 *
 * Each variant represents a distinct lexical unit in a JSON stream.
 */
typedef enum {
  TOKEN_ILLEGAL, // Represents a character or sequence we don't recognize.
  TOKEN_EOF,     // Represents the end of the input string.

  // Delimiters/Punctuators
  TOKEN_LBRACE,   // {
  TOKEN_RBRACE,   // }
  TOKEN_LBRACKET, // [
  TOKEN_RBRACKET, // ]
  TOKEN_COMMA,    // ,
  TOKEN_COLON,    // :

  // Literals
  TOKEN_STRING, // "any valid JSON string"
  TOKEN_NUMBER, // e.g., 101, -20, 3.14. The JSON spec, and thus LSP, allows
                // integers and floats. TODO: Maybe it will be easier to handle
                // each type exlicitly with a different token.
  TOKEN_TRUE,   // true
  TOKEN_FALSE,  // false
  TOKEN_NULL,   // null
} TokenType;

/**
 * @struct Token
 * @brief Represents a single lexical unit identified by the lexer.
 *
 * This struct doesn't store the token's value directly. Instead, it holds a
 * pointer to the beginning of the token in the original
 * input string and its length. This approach avoids allocating new memory for
 * every single token's literal. We just hold a "window" or a "view" of the
 * original input.
 */
typedef struct {
  TokenType type;            // The type of the token (e.g., TOKEN_STRING).
  const char *literal_start; // A pointer to the start of the token's text.
  size_t literal_length;     // The length of the token's text.
} Token;

//////////// LEXER /////////////

typedef struct Lexer Lexer;

/**
 * @brief Creates a new lexer.
 * @param input_buffer The null-terminated string to be tokenized.
 * @return A pointer to a new, heap-allocated Lexer. The caller is responsible
 * for freeing it with `lexer_free(...)`.
 */
Lexer *lexer_new(const char *input_buffer);

/**
 * @brief Frees the memory allocated for the lexer.
 * @param lexer A pointer to the Lexer to be freed.
 */
void lexer_free(Lexer *lexer);

/**
 * @brief Scans the input and returns the next token.
 * @param lexer A pointer to the Lexer.
 * @return A pointer to a new, heap-allocated Token. The caller is responsible
 * for freeing it. Returns a TOKEN_EOF token when the end is reached.
 */
Token *lexer_next_token(Lexer *lexer);

#endif // JSON_LEXER_H
