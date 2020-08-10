/**
 * File:   http_route.c
 * Author: AWTK Develop Team
 * Brief:  http_route
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2020-08-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */
#include "http_route.h"

ret_t http_route_dispatch(const http_route_entry_t* entries, uint32_t nr, http_connection_t* c) {
  uint32_t i = 0;
  ret_t ret = RET_OK;
  return_value_if_fail(entries != NULL && c != NULL, RET_BAD_PARAMS);

  for (i = 0; i < nr; i++) {
    const http_route_entry_t* iter = entries + i;
    if (c->method != iter->method) {
      continue;
    }

    if (http_route_match(iter->pattern, c->url)) {
      http_route_parse_args(iter->pattern, c->url, c->args);
      ret = iter->handle(c);
      if (ret == RET_STOP) {
        return RET_OK;
      } else if (ret == RET_OK) {
        return http_connection_send_ok(c);
      } else {
        return http_connection_send_fail(c, 500);
      }
    }
  }

  log_debug("not found: %s\n", c->url);

  return http_connection_send_fail(c, 404);
}

static const char* skip_c(const char* str, char c) {
  const char* p = str;
  while (*p && *p == c) {
    p++;
  }

  return p;
}

static const char* skip_to_c(const char* str, char c) {
  const char* p = str;
  while (*p && *p != c) {
    p++;
  }

  return p;
}

bool_t http_route_match(const char* pattern, const char* url) {
  uint32_t size = 0;
  const char* url_start = skip_c(url, '/');
  const char* url_end = skip_to_c(url_start, '/');
  const char* pattern_start = skip_c(pattern, '/');
  const char* pattern_end = skip_to_c(pattern_start, '/');

  do {
    if (*pattern_start != ':') {
      if ((url_end - url_start) != (pattern_end - pattern_start)) {
        return FALSE;
      }

      size = url_end - url_start;
      if (strncmp(url_start, pattern_start, size) != 0) {
        return FALSE;
      }
    }

    url_start = skip_c(url_end, '/');
    url_end = skip_to_c(url_start, '/');
    pattern_start = skip_c(pattern_end, '/');
    pattern_end = skip_to_c(pattern_start, '/');
  } while (*url_start && *pattern_start);

  if(*url_start == *pattern_start) {
    return TRUE;
  } else {
    return FALSE;
  }
}

ret_t http_route_parse_args(const char* pattern, const char* url, object_t* args) {
  char key[TK_NAME_LEN + 1];
  char value[MAX_PATH + 1];
  const char* url_start = skip_c(url, '/');
  const char* url_end = skip_to_c(url_start, '/');
  const char* pattern_start = skip_c(pattern, '/');
  const char* pattern_end = skip_to_c(pattern_start, '/');

  do {
    if (*pattern_start == ':') {
      if ((pattern_end - pattern_start) < sizeof(key) && (url_end - url_start) < sizeof(value)) {
        memset(key, 0x00, sizeof(key));
        memset(value, 0x00, sizeof(value));

        strncpy(key, pattern_start + 1, pattern_end - pattern_start - 1);
        strncpy(value, url_start, url_end - url_start);
        object_set_prop_str(args, key, value);
        log_debug("%s:%s\n", key, value);
      } else {
        log_warn("key or value is too large\n");
      }
    }

    url_start = skip_c(url_end, '/');
    url_end = skip_to_c(url_start, '/');
    pattern_start = skip_c(pattern_end, '/');
    pattern_end = skip_to_c(pattern_start, '/');
  } while (*url_start && *pattern_start);

  return RET_OK;
}
