#ifndef LSP_DISPATCHER_H
#define LSP_DISPATCHER_H

#include <stdint.h>

#include "libs/foundation.h"

typedef enum {
  LSP_STATUS_CONTINUE = 0,
  LSP_STATUS_EXIT = 1,
} lsp_status;

typedef lsp_status (*lsp_handler_fn)(int32_t id, fdn_string params);

typedef struct {
  const char *method;
  lsp_handler_fn handler;
} dispatch_entry;

extern dispatch_entry dispatch_table[];

// dispatch_message returns `LSP_STATUS_EXIT` if the server should stop; returns
// `LSP_STATUS_CONTINUE` otherwise. It is the primary function that drives the
// requested logic execution. It parses the parameters, prepares the response
// and sends it out to the client.
lsp_status dispatch_message(fdn_string method, bool has_id, int32_t id,
                            fdn_string params);

#endif // LSP_DISPATCHER_H
