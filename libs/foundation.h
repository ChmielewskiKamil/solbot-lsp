#ifndef FOUNDATION_H
#define FOUNDATION_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/////////////////////////////////////////////////
//                   STRINGS                   //
/////////////////////////////////////////////////

typedef struct {
  const char *string_start;
  size_t string_length;
} fdn_string;

/* `fdn_string_create_view` creates a "view" into the provided string. Returns
 * the `Foundation` library `fdn_string` type which is a view into a string. */
static inline fdn_string fdn_string_create_view(const char *string_start,
                                                size_t string_length) {
  fdn_string str;
  str.string_start = string_start;
  str.string_length = string_length;
  return str;
}

static inline bool fdn_string_is_eq_c_str(fdn_string view, const char *str) {
  size_t str_len = strlen(str);

  if (view.string_length != str_len) {
    return false;
  }

  return strncmp(view.string_start, str, view.string_length) == 0;
}

static inline bool fdn_string_is_eq(fdn_string str1, fdn_string str2) {
  if (str1.string_length != str2.string_length) {
    return false;
  }

  return strncmp(str1.string_start, str2.string_start, str1.string_length) == 0;
}

#endif // FOUNDATION_H

#ifdef FDN_IMPLEMENTATION

#endif // FDN_IMPLEMENTATION
