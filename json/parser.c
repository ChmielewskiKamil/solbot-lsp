#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "libs/foundation.h"
#include "parser.h"

// skip_json_value advances the lexer past json objects or arrays. In the case
// of the LSP when we parse the incoming request message, at first we just care
// about the `ID` and `method` fields. We can skip parsing the `params` since
// they are a big an complex objest. We only store the beginning and ending of
// the `params` so that the appropriate message handler function can later parse
// it easily.
static void skip_json_value(Lexer *lexer) {
  uint32_t depth = 0;
  Token tkn = lexer_next_token(lexer);

  while (tkn.type != TOKEN_EOF) {
    if (tkn.type == TOKEN_LBRACE || tkn.type == TOKEN_LBRACKET) {
      depth++;
    } else if (tkn.type == TOKEN_RBRACE || tkn.type == TOKEN_RBRACKET) {
      depth--;
    }

    if (depth == 0)
      return;

    tkn = lexer_next_token(lexer);
  }
}

/* TODO: What do I want from this function?
 *
 * Grab the incoming JSON payload (null terminated) and distinguish 3 things
 * inside it:
 * - the 'id' field (int or string)
 * - the 'method' field (string)
 * - the 'params' field (array or object)
 *
 * We will receive the JSON object that has the format { ... }
 * Inside the initial brackets there will be fields that we are looking for:
 * {
 *   "id": 1,                  <--- comma at the end
 *   "method": "initialize",   <--- comma at the end
 *   "jsonrpc": 2,             <--- there can be other garbage that we don't
 *                                  care about here; we will skip it
 *   "params": {...}           <--- no comma at the end
 * }                           <--- no comma at the end
 *
 * The 'id' and 'method' field are required by the `Dispatcher`. The
 * `Dispatcher` based on the received `method` will determine which handler
 * function to call.
 */
RequestMessage *parser_parse_request_message(const char *request_buffer) {
  RequestMessage *request = calloc(1, sizeof(*request));
  if (request == NULL) {
    return NULL;
  }

  Lexer lexer = lexer_new(request_buffer);
  Token token = lexer_next_token(&lexer);

  // Invalid JSON.
  if (token.type != TOKEN_LBRACE) {
    free(request);
    return NULL;
  }

  const char *KEY_METHOD = "\"method\"";
  const char *KEY_ID = "\"id\"";
  const char *KEY_PARAMS = "\"params\"";

  size_t LEN_METHOD = 8; // 2 quotation marks + 6 chars in word 'method'
  size_t LEN_ID = 4;     // 2 quotation marks + 2 chars in word 'id'
  size_t LEN_PARAMS = 8; // 2 quotation marks + 6 chars in word 'params'

  while (1) {
    token = lexer_next_token(&lexer);
    if (token.type != TOKEN_STRING) {
      free(request);
      return NULL;
    }

    int8_t is_method = 0;
    int8_t is_id = 0;
    int8_t is_params = 0;

    if (token.literal_length == LEN_METHOD &&
        strncmp(KEY_METHOD, token.literal_start, LEN_METHOD) == 0) {
      is_method = 1;
    }

    else if (token.literal_length == LEN_ID &&
             strncmp(KEY_ID, token.literal_start, LEN_ID) == 0) {
      is_id = 1;
    }

    else if (token.literal_length == LEN_PARAMS &&
             strncmp(KEY_PARAMS, token.literal_start, LEN_PARAMS) == 0) {
      is_params = 1;
    }

    token = lexer_next_token(&lexer);
    if (token.type != TOKEN_COLON) {
      free(request);
      return NULL;
    }

    if (is_method) {
      token = lexer_next_token(&lexer); // Move past the colon.

      if (token.type != TOKEN_STRING) {
        free(request);
        return NULL;
      }

      // Skip the opening and closing quotation marks e.g.
      // str:  " i n i t "
      // idx:  0 1 2 3 4 5
      //
      // The length of the method name is 4.
      // The length of the original string is 6 (from 0 to 5)
      // Start  + 1 -> points at idx 1
      // Length - 2 -> points at idx 4
      request->method = fdn_string_create_view(token.literal_start + 1,
                                               token.literal_length - 2);

    } else if (is_id) {
      token = lexer_next_token(&lexer); // Move past the colon.

      if (token.type == TOKEN_NUMBER) {
        request->id = (int32_t)(strtol(token.literal_start, NULL, 10));
        request->has_id = true; // Notifications don't have IDs; we are
                                // dealing with a request message.
      }
      // TODO: Handle IDs that are strings
    } else if (is_params) {
      // We are sitting at the COLON right now. We do not call the
      // `lexer_next_token` since `skip_json_value` will do it for us as it
      // advances the `Lexer` state as its first operation.
      //
      // IMPORTANT: While the current token type is `COLON`, the `Lexer`'s
      // internal position has been updated and already points at the next
      // `char`. In our case it already points at the opening `LBRACE`. This
      // mechanism stems from the design approach taken in the
      // `lexer_next_token` function.
      const char *start = lexer.position;

      skip_json_value(&lexer);

      const char *end = lexer.position;

      request->params = fdn_string_create_view(start, (size_t)(end - start));
    } else {
      // Skip unknown JSON keys e.g. "jsonrpc", as we don't care about that.
      skip_json_value(&lexer);
    }

    token = lexer_next_token(&lexer); // Should be either COMMA or RBRACE.

    // Case: We finished parsing the incoming request message. Handle happy exit
    // case first.
    if (token.type == TOKEN_RBRACE) {
      return request;
    }

    // Case: In between the JSON key-value pairs there should be a COMMA. At the
    // very end for the last key-value pair, we should enter the earlier if
    // statement.
    if (token.type != TOKEN_COMMA) {
      free(request);
      return NULL;
    }
  }
}
