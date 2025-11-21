#ifndef LSP_DISPATCHER_H
#define LSP_DISPATCHER_H

#include <stdint.h>

typedef void (*LspHandler) (int32_t id, const char *params);

typedef struct {
    const char *method;
    LspHandler handler;
} dispatch_entry_t;

extern dispatch_entry_t dispatch_table[];

void dispatch_message(const char *method, int32_t id, const char *params);

#endif // LSP_DISPATCHER_H
