#include <stdio.h>

// Forward declare your test functions
void test_lexer_simple_tokens();
void test_parser_handles_empty_input();
// Add more test declarations here...

// A simple struct to hold test results
typedef struct {
  int passed;
  int failed;
} TestStats;

// Helper to run a single test
void run_test(void (*test_function)(), const char *test_name,
              TestStats *stats) {
  // This is a simple trick to check for crashes (segfaults)
  // A more robust solution would use fork() and waitpid()
  printf("--- Running: %s ---\n", test_name);
  test_function(); // Call the test
  printf("--- Passed: %s ---\n\n", test_name);
  stats->passed++;
  // Note: If a segfault occurs, the program will crash here and not proceed.
}

int main() {
  TestStats stats = {0, 0};

  printf("========= Starting Tests =========\n\n");

  // Add your tests to be run here
  run_test(test_lexer_simple_tokens, "test_lexer_simple_tokens", &stats);
  run_test(test_parser_handles_empty_input, "test_parser_handles_empty_input",
           &stats);

  printf("========= Test Summary =========\n");
  printf("Passed: %d, Failed: %d\n", stats.passed, stats.failed);
  printf("================================\n");

  return stats.failed > 0 ? 1 : 0;
}

// --- Include C files to be tested ---
#include "json/lexer.c"
#include "json/parser.c"

// --- Assertion Helper ---
// A basic assert to check conditions in tests.
void assert_true(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "Assertion failed: %s\n", message);
    // In a real test, you'd increment a failure count.
    // For this simple runner, we'll just exit.
    exit(1);
  }
}

// --- Test Implementations ---

void test_lexer_simple_tokens() {
  const char *input = "{}";
  Lexer *lexer = lexer_new(input);
  assert_true(lexer != NULL, "Lexer should not be null.");

  Token *token = lexer_next_token(lexer);
  assert_true(token->type == TOKEN_LBRACE, "First token should be LBRACE.");
  free(token);

  token = lexer_next_token(lexer);
  assert_true(token->type == TOKEN_RBRACE, "Second token should be RBRACE.");
  free(token);

  token = lexer_next_token(lexer);
  assert_true(token->type == TOKEN_EOF, "Third token should be EOF.");
  free(token);

  lexer_free(lexer);
}

void test_parser_handles_empty_input() {
  // This test ensures the fixes prevent a crash on empty/invalid input
  const char *input = "";
  RequestMessage *req = parser_parse_request_message(input);
  assert_true(
      req != NULL,
      "Parser should return a valid request pointer even on empty input.");
  free(req);
}
