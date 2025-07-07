#include "parser.h"
#include "lexer.h"
#include <stdlib.h>

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

RequestMessage *parser_parse_request_message(const char *request_buffer) {
  RequestMessage *request = NULL;
  Parser *parser = parser_new(request_buffer);
  if (parser == NULL) {
    return NULL;
  }

  // TODO: When to release Lexer?
  // TODO: Implement parser_free and lexer_free functions.
  free(parser);
  return request;
}
