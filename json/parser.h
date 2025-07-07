#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdint.h>

typedef enum {
  M_INITIALIZE, // LSP: 'initialize'
  M_SHUTDOWN,   // LSP: 'shutdown'
  M_EXIT,       // LSP: 'exit'
} RequestMethod;

// RequestMessage from the Language Server Protocol (LSP) specification. It is
// sent by the client (the code editor) to the server (this program).
typedef struct {
  int32_t id;
  RequestMethod method;
  void *params;
} RequestMessage;

RequestMessage *parser_parse_request_message(const char *request_buffer);

#endif // JSON_PARSER_H
