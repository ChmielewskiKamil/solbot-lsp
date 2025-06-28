#include <stdio.h>
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
  // 'fopen' with write mode "w" clears the file; 'fclose' immediately
  // closes it. The end result is an empty solbot-lsp.log file on each launch.
  fclose(fopen("/tmp/solbot-lsp.log", "w")); 
  log_message("--- Solbot LSP Started ---");

  char line_buffer[1024];
  char *separator = "\r\n";

  while (1) {
    if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
      break;
    }

    log_message(line_buffer);

    if (strcmp(line_buffer, separator) == 0) {
      log_message("Found the end of header section");
      break;
    }
  }

  return 0;
}
