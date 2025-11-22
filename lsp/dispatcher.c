#include "dispatcher.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/////// LSP REQUEST MESSAGE HANDLERS - FORWARD DECLARATIONS ///////

void handle_initialize(int32_t id, const char *params);

dispatch_entry_t dispatch_table[] = {
    {"initialize", handle_initialize},
    {NULL, NULL} // sentinel value marks the end of loop iteration over the
                 // dispatch table
};

void dispatch_message(const char *method, int32_t id, const char *params) {
  for (int32_t i = 0; dispatch_table[i].method != NULL; i++) {
    if (strcmp(method, dispatch_table[i].method) == 0) {
      dispatch_table[i].handler(id, params);
      return;
    }
  }
}

void handle_initialize(int32_t id, const char *params) {
  const char *response = "{"
                         "\"jsonrpc\": \"2.0\","
                         "\"id\": 1,"
                         "\"result\": {"
                         "  \"capabilities\": {}"
                         "}"
                         "}";

  printf("Content-Length: %zu\r\n", strlen(response));
  printf("\r\n");
  printf("%s", response);
  fflush(stdout);

  // TODO: Parse the initialize request meessage
  //
  // TODO: Prepare a response and send it out
}
