#include <dispatcher.h>

#include "string.h"

dispatch_entry_t dispatch_table[] = {
    {NULL, NULL} // sentinel value marks the end of loop iteration over the dispatch table
};

void dispatch_message(const char *method, int32_t id, const char *params) {
    for (int32_t i = 0; dispatch_table[i].method != NULL; i++) {
        if (strcmp(method, dispatch_table[i].method) == 0) {
            dispatch_table[i].handler(id, params);
            return;
        }
    }
}
