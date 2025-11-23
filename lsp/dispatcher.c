#include "dispatcher.h"

#include <foundation.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/////// LSP REQUEST MESSAGE HANDLERS - FORWARD DECLARATIONS ///////

void handle_initialize(int32_t id, fdn_string params);
void handle_shutdown(int32_t id, fdn_string params);

/// LSP NOTIFICATIONS - FORWARD DECLARATIONS ///

// TODO: End the process; should exit with status 0 if there was shutdown
// request before. If there was no shutdown request, it should exit with
// status 1.
void handle_exit(int32_t id, fdn_string params);

///////////////////////////////////////////////////
///////// DISPATCHER - LSP MESSAGE ROUTER /////////
///////////////////////////////////////////////////

dispatch_entry_t dispatch_table[] = {
    {"initialize", handle_initialize},
    {"shutdown", handle_shutdown},
    {"exit", handle_exit}, 
    {NULL, NULL} // sentinel value marks the end of loop iteration over the
                 // dispatch table
};

void dispatch_message(fdn_string method, int32_t id, fdn_string params) {
  for (int32_t i = 0; dispatch_table[i].method != NULL; i++) {
    if (fdn_string_is_eq_c_str(method, dispatch_table[i].method)) {
      dispatch_table[i].handler(id, params);
      return;
    }
  }
}

/////// LSP REQUEST MESSAGE HANDLERS - IMPLEMENTATIONS ///////

void handle_initialize(int32_t id, fdn_string params) {
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

void handle_shutdown(int32_t id, fdn_string params) {
  const char *response = "{"
                         "\"jsonrpc\": \"2.0\","
                         "\"id\": 2,"
                         "\"result\":null"
                         "}";

  printf("Content-Length: %zu\r\n", strlen(response));
  printf("\r\n");
  printf("%s", response);
  fflush(stdout);
}

void handle_exit(int32_t id, fdn_string params) {
   // TODO: This should somehow track whether shutdown request was received and
   // then signal exit to the process.
   return; 
}
