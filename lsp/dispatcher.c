#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatcher.h"
#include "libs/foundation.h"

// --- Global State ---
static bool g_shutdown_requested = 0;

/////// LSP REQUEST MESSAGE HANDLERS - FORWARD DECLARATIONS ///////

lsp_status handle_initialize(int32_t id, fdn_string params);
lsp_status handle_shutdown(int32_t id, fdn_string params);

/// LSP NOTIFICATIONS - FORWARD DECLARATIONS ///

// TODO: End the process; should exit with status 0 if there was shutdown
// request before. If there was no shutdown request, it should exit with
// status 1.
lsp_status handle_exit(int32_t id, fdn_string params);

///////////////////////////////////////////////////
///////// DISPATCHER - LSP MESSAGE ROUTER /////////
///////////////////////////////////////////////////

dispatch_entry dispatch_table[] = {
    {"initialize", handle_initialize},
    {"shutdown", handle_shutdown},
    {"exit", handle_exit},
    {NULL, NULL} // sentinel value marks the end of loop iteration over the
                 // dispatch table
};

lsp_status dispatch_message(fdn_string method, bool has_id, int32_t id,
                            fdn_string params) {
  for (int32_t i = 0; dispatch_table[i].method != NULL; i++) {
    if (fdn_string_is_eq_c_str(method, dispatch_table[i].method)) {
      lsp_status should_exit = dispatch_table[i].handler(id, params);
      return should_exit;
    }
  }

  // TODO: Handle the case where a method was not found. This probably requires
  // implementing:
  // - send error function similar to send response
  // - sending a specific error code in main (or here???)
  // - LSP should still work but we should notify the client
  return LSP_STATUS_CONTINUE;
}

// `send_response` prepares a response message by constructing the full JSON
// payload using the provided `id` and `json_result`. The function returns `0`
// on success and `-1` on error. The provided `json_result` MUST be a valid JSON
// object. The `send_response` function constructs the message by appending the
// regular "jsonrpc", "id" and "result" fields required by the LSP. The
// `json_result` is placed as a value of the `result` field.
static int send_response(int32_t id, const char *json_result) {
  size_t result_len = strlen(json_result);
  // Calculate the length required for the specialized JSON result to be sent
  // back to the client + a small overhead of 64 bytes for the constant portion
  // of each response message (the "jsonrpc", "id", "result" fields etc.).
  size_t required_len = result_len + 64;

  char stack_buffer[2048];
  char *buffer = stack_buffer; // default to stack buffer for small mesages

  if (required_len > sizeof(stack_buffer)) {
    buffer = malloc(required_len);
    if (!buffer)
      return -1; // allocation error; server should most likely treat this as a
                 // sign to exit.
  }

  // Write the formatted JSON response into the buffer.
  int response_len = snprintf(buffer, required_len,
                              "{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":%s}",
                              id, json_result);

  // Append the Content-Length header to the response and send it to `stdout`.
  if (fprintf(stdout, "Content-Length: %d\r\n\r\n%s", response_len, buffer) <
      0) {
    if (buffer != stack_buffer)
      free(buffer);
    return -1;
  }

  if (fflush(stdout) == EOF) {
    if (buffer != stack_buffer)
      free(buffer);
    return -1;
  }

  if (buffer != stack_buffer) {
    free(buffer);
  }

  return 0; // success
}

//////////////////////////////////////////////////////////////
/////// LSP REQUEST MESSAGE HANDLERS - IMPLEMENTATIONS ///////
//////////////////////////////////////////////////////////////

lsp_status handle_initialize(int32_t id, fdn_string params) {
  (void)params;

  // TODO: Parse the initialize request meessage
  
  const char *result = "{\"capabilities\":{}}";

  if (send_response(id, result) == -1) {
    return LSP_STATUS_EXIT;
  }

  return LSP_STATUS_CONTINUE;
}

lsp_status handle_shutdown(int32_t id, fdn_string params) {
  (void)params;
  g_shutdown_requested = true;

  const char *response = "{"
                         "\"jsonrpc\": \"2.0\","
                         "\"id\": 2,"
                         "\"result\":null"
                         "}";

  printf("Content-Length: %zu\r\n", strlen(response));
  printf("\r\n");
  printf("%s", response);
  fflush(stdout);
  send_response(id, "null");

  return LSP_STATUS_CONTINUE;
}

lsp_status handle_exit(int32_t id, fdn_string params) {
  (void)id;
  (void)params;

  // TODO: This should somehow track whether shutdown request was received and
  // then signal exit to the process.
  return LSP_STATUS_EXIT;
}
