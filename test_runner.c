#include "json/lexer.h"
#include <stdio.h>

// Forward declaration of test case functions.
int test_lexer_simple_tokens(void);
int test_parser_handles_empty_input(void);

// A simple struct to hold test results
typedef struct {
  int passed;
  int failed;
} TestStats;

// A helper function that runs a single test case.
void run_test(int (*test_function)(void), const char *test_name,
              TestStats *stats) {
  printf("--- Running: %s ---\n", test_name);

  // Call the test and check if it passed or failed.
  if (test_function()) {
    printf("--- \033[38;2;50;255;50mPassed\033[0m: %s ---\n\n", test_name);
    stats->passed++;
  } else {
    // The assertion already printed the detailed error.
    printf("--- \033[38;2;255;50;50mFailed\033[0m: %s ---\n\n", test_name);
    stats->failed++;
  }
}

int main(void) {
  TestStats stats = {0, 0};

  printf("========= Starting Tests =========\n\n");

  run_test(test_lexer_simple_tokens, "test_lexer_simple_tokens", &stats);
  run_test(test_parser_handles_empty_input, "test_parser_handles_empty_input",
           &stats);

  printf("========== Test Summary ==========\n");
  printf("Passed: %d, Failed: %d\n", stats.passed, stats.failed);
  printf("==================================\n");

  return stats.failed > 0 ? 1 : 0;
}

// --- Include C files to be tested ---
#include "json/lexer.c"
#include "json/parser.c"

// --- Test Helpers ---

// Converts a TokenType enum to a string for readable test output.
const char *token_type_to_string(TokenType type) {
  switch (type) {
  case TOKEN_ILLEGAL:
    return "TOKEN_ILLEGAL";
  case TOKEN_EOF:
    return "TOKEN_EOF";
  case TOKEN_LBRACE:
    return "TOKEN_LBRACE";
  case TOKEN_RBRACE:
    return "TOKEN_RBRACE";
  case TOKEN_LBRACKET:
    return "TOKEN_LBRACKET";
  case TOKEN_RBRACKET:
    return "TOKEN_RBRACKET";
  case TOKEN_COMMA:
    return "TOKEN_COMMA";
  case TOKEN_COLON:
    return "TOKEN_COLON";
  case TOKEN_STRING:
    return "TOKEN_STRING";
  case TOKEN_NUMBER:
    return "TOKEN_NUMBER";
  case TOKEN_TRUE:
    return "TOKEN_TRUE";
  case TOKEN_FALSE:
    return "TOKEN_FALSE";
  case TOKEN_NULL:
    return "TOKEN_NULL";
  default:
    return "UNKNOWN_TOKEN";
  }
}

// --- Assertion Helpers ---

// A basic assert to check conditions in tests.
void assert_true(int condition, const char *message) {
  if (!condition) {
    fprintf(stderr, "Assertion failed: %s\n", message);
    exit(1);
  }
}

// Returns 1 on success, 0 on failure.
int assert_token_type(TokenType expected, TokenType actual, int test_case_num) {
  if (expected == actual) {
    return 1; // Success
  }

  fprintf(stderr, "[Test Case %d] Assertion Failed: Expected %s, but got %s.\n",
          test_case_num, token_type_to_string(expected),
          token_type_to_string(actual));

  return 0; // Failure
}

// --- Test Implementations ---

int test_lexer_simple_tokens(void) {
  const char *input = "{}true,false,null:";
  int success = 1; // Assume the test will pass

  struct TestCase {
    TokenType expected_type;
  } tests[] = {
      {TOKEN_LBRACE}, {TOKEN_RBRACE}, {TOKEN_TRUE}, {TOKEN_FALSE},
      {TOKEN_NULL},   {TOKEN_COLON},  {TOKEN_EOF},
  };

  Lexer *lexer = lexer_new(input);
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  for (int i = 0; i < num_tests; i++) {
    Token *tok = lexer_next_token(lexer);

    // Check if the assertion failed.
    if (!assert_token_type(tests[i].expected_type, tok->type, i + 1)) {
      success = 0; // Mark the test as failed.
      free(tok);   // Free the token that caused the failure.
      break;       // Exit the test loop.
    }

    free(tok);
  }

  lexer_free(lexer);

  return success; // Return the final status.
}

int test_parser_handles_empty_input(void) {
  const char *input = "";
  RequestMessage *req = parser_parse_request_message(input);

  if (req == NULL) {
    fprintf(stderr, "Assertion Failed: parser_parse_request_message returned "
                    "NULL on empty input.\n");
    return 0; // Failure
  }

  // If we reach here, the test passed.
  free(req);
  return 1; // Success
}
