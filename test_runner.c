#include <foundation.h>
#include <json/parser.h>
#include <stdio.h>
#include <stdlib.h>

// --- Project Includes (Unity Build Style) ---
// We include the .c files directly so we can test static functions if needed
#include "json/lexer.c"
#include "json/parser.c"

// ==============================================================================
// 1. THE TESTING FRAMEWORK (The Engine)
// ==============================================================================

typedef struct {
    int passed;
    int failed;
} TestStats;

// Helper to convert TokenType to string
const char *token_type_str(TokenType type) {
    switch (type) {
        case TOKEN_ILLEGAL: return "ILLEGAL";
        case TOKEN_EOF:     return "EOF";
        case TOKEN_LBRACE:  return "LBRACE";
        case TOKEN_RBRACE:  return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA:   return "COMMA";
        case TOKEN_COLON:   return "COLON";
        case TOKEN_STRING:  return "STRING";
        case TOKEN_NUMBER:  return "NUMBER";
        case TOKEN_TRUE:    return "TRUE";
        case TOKEN_FALSE:   return "FALSE";
        case TOKEN_NULL:    return "NULL";
        default:            return "UNKNOWN";
    }
}

// --- ASSERTION MACROS ---

#define ASSERT_TRUE(condition, msg) \
    if (!(condition)) { \
        fprintf(stderr, "    [ASSERT FAILED] %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 0; \
    }

#define ASSERT_TOKEN(expected, actual) \
    if ((expected) != (actual)) { \
        fprintf(stderr, "    [ASSERT FAILED] %s:%d: Expected %s, got %s\n", \
                __FILE__, __LINE__, token_type_str(expected), token_type_str(actual)); \
        return 0; \
    }

#define ASSERT_NOT_NULL(ptr, msg) \
    if ((ptr) == NULL) { \
        fprintf(stderr, "    [ASSERT FAILED] %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 0; \
    }

#define ASSERT_NULL(ptr, msg) \
    if ((ptr) != NULL) { \
        fprintf(stderr, "    [ASSERT FAILED] %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 0; \
    }

// --- RUNNER MACROS ---

// The internal runner function
void run_test_internal(int (*test_func)(void), const char *name, TestStats *stats) {
    printf("--- Running: %s ---\n", name);
    
    if (test_func()) {
        printf("--- \033[38;2;50;255;50mPassed\033[0m: %s ---\n\n", name); // Green
        stats->passed++;
    } else {
        printf("--- \033[38;2;255;50;50mFailed\033[0m: %s ---\n\n", name); // Red
        stats->failed++;
    }
}

// #func turns the function name into a string literal automatically
#define RUN_TEST(func) run_test_internal(func, #func, &stats)


// ==============================================================================
// 2. THE TEST CASES
// ==============================================================================

int test_lexer_simple_tokens(void) {
    // Note the double backslash to actually put a backslash in the C-string
    const char *input = "{} \"hello\" \"hello with quote \\\"mark \"";
    
    struct TestCase { TokenType expected; } tests[] = {
        {TOKEN_LBRACE}, {TOKEN_RBRACE}, {TOKEN_STRING}, {TOKEN_STRING}, {TOKEN_EOF},
    };

    Lexer lexer = lexer_new(input);
    int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++) {
        Token tok = lexer_next_token(&lexer);
        ASSERT_TOKEN(tests[i].expected, tok.type);
    }

    return 1; // Success
}

int test_parser_returns_null_on_empty_input(void) {
    const char *input = " "; // Whitespace only
    RequestMessage *req = parser_parse_request_message(input);
    
    // Check Result
    ASSERT_NULL(req, "Parser should return NULL on empty/whitespace input");
    
    return 1;
}

int test_parser_handles_method_parsing(void) {
    const char *input = "{\"method\": \"initialize\"}";
    RequestMessage *req = parser_parse_request_message(input);

    ASSERT_NOT_NULL(req, "Parser returned NULL on valid input");
    ASSERT_TRUE(req->method.string_length > 0, "Method should be parsed");

    fdn_string expected_method_name = fdn_string_create_view("initialize", 10);

    ASSERT_TRUE(fdn_string_is_eq(req->method, expected_method_name), "Method name mismatch.");
    
    free(req);
    return 1;
}

int test_parser_handles_id_parsing(void) {
    const char *input = "{\"id\": 10 }";
    RequestMessage *req = parser_parse_request_message(input);
    ASSERT_NOT_NULL(req, "Parser returned NULL on valid input");

    ASSERT_TRUE(req->id == 10, "Request message ID mismatch.");

    free(req);
    return 1;
}

// ==============================================================================
// 3. MAIN ENTRY POINT
// ==============================================================================

int main(void) {
    TestStats stats = {0, 0};

    printf("========= Starting Tests =========\n\n");

    RUN_TEST(test_lexer_simple_tokens);
    RUN_TEST(test_parser_returns_null_on_empty_input);
    RUN_TEST(test_parser_handles_method_parsing);
    RUN_TEST(test_parser_handles_id_parsing);

    printf("========== Test Summary ==========\n");
    printf("Passed: %d, Failed: %d\n", stats.passed, stats.failed);
    printf("==================================\n");

    return stats.failed > 0 ? 1 : 0;
}
