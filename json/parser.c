#include "parser.h"
#include "lexer.h"
#include <assert.h>
#include <stdlib.h>

typedef struct {
  Lexer *lexer;
  Token *tkn_current;
  Token *tkn_peek;
} Parser;

void parser_next_token(Parser *parser) {
  parser->tkn_current = parser->tkn_peek;
  parser->tkn_peek = lexer_next_token(parser->lexer);
}

Parser *parser_new(const char *request_buffer) {
  Parser *parser = NULL;

  Lexer *lexer = lexer_new(request_buffer);
  if (lexer == NULL) {
    return NULL;
  }

  parser = malloc(sizeof(*parser));
  if (parser == NULL) {
    return NULL;
  }

  parser->lexer = lexer;

  parser_next_token(parser);
  parser_next_token(parser);

  return parser;
}

void parser_free(Parser *parser) {
  assert(parser != NULL);
  assert(parser->tkn_current != NULL);
  assert(parser->tkn_peek != NULL);

  lexer_free(parser->lexer);

  free(parser->tkn_current);
  free(parser->tkn_peek);

  free(parser);
}

RequestMessage *parser_parse_request_message(const char *request_buffer) {
  RequestMessage *request = malloc(sizeof(*request));
  if (request == NULL) {
    return NULL;
  }

  Parser *parser = parser_new(request_buffer);
  if (parser == NULL) {
    return NULL;
  }

  parser_free(parser);
  return request;
}
