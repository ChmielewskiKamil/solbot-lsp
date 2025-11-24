// --- Standard Headers ---

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Project Headers ---
// Ensure that all declarations are known before the implementation 'c' files
// are compiled for the unity build.

#include "libs/foundation.h" // the "Base Layer"; custom standard library
#define FDN_IMPLEMENTATION

#include "lsp/dispatcher.h"
#include "json/parser.h"

// --- Unity Build ---

#include "lsp/dispatcher.c"
#include "json/lexer.c"
#include "json/parser.c"

/**
 * @brief Safely appends a single message to the persistent LSP log file.
 *
 * @details WHY this function exists:
 * In a Language Server, stdout is reserved for protocol messages to the editor.
 * Therefore, a separate channel is needed for debugging. This function provides
 * that channel by writing to a file in /tmp, a standard location for transient
 * data.
 *
 * WHY it uses append ('a') mode:
 * To build a sequential log of events during a session. If it used write ('w')
 * mode, each new message would erase the entire log, making it useless.
 *
 * WHY it opens and closes the file on every call:
 * This is a trade-off for durability. Closing the file forces the system to
 * immediately flush the write buffer to disk. This ensures that even if the LSP
 * crashes right after logging, the last message is very likely to be saved.
 * The alternative (keeping the file open globally) would be more performant but
 * would risk losing recent logs in a crash.
 */
void log_message(const char *message) {
  // TODO: Move this function to logger.h and logger.c so that it can be
  // accessed from any file.

  FILE *log_file = fopen("/tmp/solbot-lsp.log", "a"); // 'a' - append mode
  if (log_file != NULL) {
    fputs(message, log_file);
    fputc('\n', log_file);
    fclose(log_file);
  }
}

int main(void) {
  FILE *log_file = fopen("/tmp/solbot-lsp.log", "w");
  if (log_file == NULL) {
    return 1;
  }

  fdn_log_init(log_file);

  fdn_info("--- Solbot LSP Started ---");

  char *separator = "\r\n";
  char line_buffer[1024];

  while (1) {
    uint32_t content_length = 0; // Reset for each message

    // --- Header Part ---

    while (fgets(line_buffer, sizeof(line_buffer), stdin) != NULL) {
      if (strncmp(line_buffer, "Content-Length: ", 16) == 0) {
        sscanf(line_buffer + 16, "%" PRIu32, &content_length);
        char content_len_log[70];
        sprintf(content_len_log,
                "[Info] Found content section length header; len: %" PRIu32,
                content_length);
      }

      if (strcmp(line_buffer, separator) == 0) {
        break;
      }
    }

    if (content_length == 0) {
      // TODO: Can client -> server notifications have 0 length?
      return 1;
    }

    // --- Content Part

    // Add +1 for null terminator.
    char *content_buffer = malloc(content_length + 1);
    if (content_buffer == NULL) {
      return 1;
    }

    size_t bytes_read =
        fread(content_buffer, sizeof(char), content_length, stdin);

    if (bytes_read != content_length) {
      free(content_buffer);
      return 1;
    }

    content_buffer[content_length] = '\0'; // 'fread' does not null-terminate

    fdn_info("Raw request message: %s", content_buffer);

    RequestMessage *request = parser_parse_request_message(content_buffer);
    if (request == NULL) {
      free(content_buffer);
      return 1;
    }

    fdn_info("Dispatching method: %.*s", (int)request->method.string_length,
             request->method.string_start);

    lsp_status status = dispatch_message(request->method, request->has_id,
                                         request->id, request->params);

    free(request);
    free(content_buffer);

    if (status == LSP_STATUS_EXIT) {
      fdn_info("Exit signal received. Shutting down.");
      break;
    }
  }

  return 0;
}
