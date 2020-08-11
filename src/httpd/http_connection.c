/**
 * File:   http_connection.c
 * Author: AWTK Develop Team
 * Brief:  http_connection
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

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/object_default.h"

#include "http_connection.h"

http_connection_t* http_connection_create(tk_iostream_t* io, int32_t method, const char* url,
                                          const char* body, uint32_t body_size) {
  http_connection_t* c = NULL;
  return_value_if_fail(io != NULL && url != NULL, NULL);

  c = TKMEM_ZALLOC(http_connection_t);
  return_value_if_fail(c != NULL, NULL);

  c->io = io;
  c->url = url;
  c->method = method;
  c->args = object_default_create();
  c->resp = conf_doc_load_json("{}", 2);

  if (body != NULL) {
    c->req = conf_doc_load_json(body, body_size);
  }

  return c;
}

ret_t http_connection_send_ok(http_connection_t* c) {
  int ret = 0;
  str_t str;
  str_t str_body;
  char header[256];
  str_t* body = &(str_body);
  tk_ostream_t* out = tk_iostream_get_ostream(c->io);

  str_init(&str, 512);
  str_init(&str_body, 512);

  conf_doc_save_json(c->resp, body);
  tk_snprintf(header, sizeof(header),
              "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\n"
              "Cache-Control: no-cache\r\nContent-Length: %d\r\n\r\n",
              body->size);

  ENSURE(str_set(&str, header) == RET_OK);
  ENSURE(str_append(&str, body->str) == RET_OK);
  ret = tk_ostream_write_len(out, str.str, str.size, 0);
  ENSURE(ret == str.size);

  log_debug("%d:%s\n", ret, str.str);
  str_reset(&str);
  str_reset(&str_body);

  return RET_OK;
}

ret_t http_connection_send_fail(http_connection_t* c, int32_t status_code) {
  int ret = 0;
  char header[256];
  tk_ostream_t* out = tk_iostream_get_ostream(c->io);
  return_value_if_fail(c != NULL, RET_BAD_PARAMS);

  tk_snprintf(header, sizeof(header), "HTTP/1.1 %d Server Error\r\n"
      "Cache-Control: no-cache\r\nContent-Length: 0\r\n\r\n",
      status_code);
  ret = tk_ostream_write_len(out, header, strlen(header), 0);
  log_debug("%d:%s\n", ret, header);

  return RET_OK;
}

ret_t http_connection_destroy(http_connection_t* c) {
  return_value_if_fail(c != NULL, RET_OK);

  OBJECT_UNREF(c->args);
  if (c->req != NULL) {
    conf_doc_destroy(c->req);
  }
  if (c->resp != NULL) {
    conf_doc_destroy(c->resp);
  }
  memset(c, 0x00, sizeof(*c));
  TKMEM_FREE(c);

  return RET_OK;
}
