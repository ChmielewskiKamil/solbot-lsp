#ifndef LSP_DISPATCHER_H
#define LSP_DISPATCHER_H

#include <stdint.h>

#include "libs/foundation.h"

typedef void (*LspHandler) (int32_t id, fdn_string params);

typedef struct {
    const char *method;
    LspHandler handler;
} dispatch_entry_t;

extern dispatch_entry_t dispatch_table[];

void dispatch_message(fdn_string method, int32_t id, fdn_string params);

#endif // LSP_DISPATCHER_H
