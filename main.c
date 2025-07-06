#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Safely appends a single message to the persistent LSP log file.
 *
 * WHY this function exists:
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
  FILE *log_file = fopen("/tmp/solbot-lsp.log", "a"); // 'a' - append mode
  if (log_file != NULL) {
    fputs(message, log_file);
    fputc('\n', log_file);
    fclose(log_file);
  }
}

int main() {
  fclose(fopen("/tmp/solbot-lsp.log", "w"));
  log_message("[Info] --- Solbot LSP Started ---");

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
        log_message(content_len_log);
      }

      if (strcmp(line_buffer, separator) == 0) {
        log_message("[Info] --- Headers Received ---");
        break;
      }
    }

    if (content_length == 0) {
      // TODO: Can client -> server notifications have 0 length?
      log_message("[Info] Content section length in the header is 0.");
      return 1;
    }

    // --- Content Part

    char *content_buffer = malloc(content_length + 1);
    if (content_buffer == NULL) {
      log_message("[Error] Could not allocate memory for content buffer.");
      return 1;
    }

    size_t bytes_read =
        fread(content_buffer, sizeof(char), content_length, stdin);

    if (bytes_read != content_length) {
      log_message("[Error] Could not read full message body.");
      free(content_buffer);
      return 1;
    }

    content_buffer[content_length] = '\0'; // 'fread' does not null-terminate

    log_message(content_buffer);
    log_message("[Info] --- Content Received ---");

    if (strstr(content_buffer, "\"method\":\"initialize\"") != NULL) {
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

      log_message("[Info] --- Sent 'initialize' Response ---");
    }

    if (strstr(content_buffer, "\"method\":\"shutdown\"") != NULL) {
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

    if (strstr(content_buffer, "\"method\":\"exit\"") != NULL) {
      free(content_buffer);
      return 0;
    }

    free(content_buffer);
  }

  return 0;
}
