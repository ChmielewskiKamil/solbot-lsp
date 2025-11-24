#ifndef FOUNDATION_H
#define FOUNDATION_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

/////////////////////////////////////////////////
//                   LOGGER                    //
/////////////////////////////////////////////////

/**
 * To use the logger, first initialize it with `fdn_log_file_init` function.
 * */

typedef enum {
  FDN_LOG_INFO,
  FDN_LOG_ERROR,
} fdn_log_level;

void fdn_log_init(FILE *file);
void fdn_log_message(fdn_log_level level, const char *tag, const char *file,
                     int line, const char *fmt, ...)
    __attribute__((
        format(printf, 5,
               6))); // Tell the compiler that the fmt string is the 5th param
                     // and start checking types from 6th param onwards.

#ifndef FDN_LOG_TAG
#define FDN_LOG_TAG "DEFAULT"
#endif

#define fdn_info(...)                                                          \
  fdn_log_message(FDN_LOG_INFO, FDN_LOG_TAG, __FILE__, __LINE__, __VA_ARGS__)
#define fdn_error(...)                                                         \
  fdn_log_message(FDN_LOG_ERROR, FDN_LOG_TAG, __FILE__, __LINE__, __VA_ARGS__)

#endif // FOUNDATION_H

#ifdef FDN_IMPLEMENTATION
#ifndef FDN_IMPLEMENTATION_ONCE
#define FDN_IMPLEMENTATION_ONCE

/////////////////////////////////////////////////
//                   LOGGER                    //
/////////////////////////////////////////////////

static FILE *g_fdn_log_file = NULL;

void fdn_log_init(FILE *file) { g_fdn_log_file = file; }

void fdn_log_message(fdn_log_level level, const char *tag, const char *file,
                     int line, const char *fmt, ...) {
  FILE *stream = g_fdn_log_file ? g_fdn_log_file : stderr;

  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  char time_buffer[20];
  strftime(time_buffer, 20, "%H:%M:%S", tm_info);

  const char *level_str = (level == FDN_LOG_ERROR) ? "[ERROR]" : "[INFO]";

  // Get a stripped file path e.g. from nix/store/.../main.c -> /main.c
  const char *short_file = strrchr(file, '/');
  // +1 to skip the slash e.g. /main.c -> main.c
  short_file = short_file ? short_file + 1 : file;

  fprintf(stream, "%s %s [%s] %s:%d: ", level_str, time_buffer, tag, short_file,
          line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stream, fmt, args);
  va_end(args);

  fprintf(stream, "\n");
  fflush(stream);
}

#endif // FDN_IMPLEMENTATION_ONCE
#endif // FDN_IMPLEMENTATION
