#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdint.h>

#include "libs/foundation.h"

// RequestMessage from the Language Server Protocol (LSP) specification. It is
// sent by the client (the code editor) to the server (this program).
typedef struct {
  int32_t id;
  int8_t isNotification; // notification messages have no ID field; 1 if the message is the notification; 0 if the message is a request
  fdn_string method;
  fdn_string params;
} RequestMessage;

RequestMessage *parser_parse_request_message(const char *request_buffer);

#endif // JSON_PARSER_H
