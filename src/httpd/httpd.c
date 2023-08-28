/**
 * File:   httpd.h
 * Author: AWTK Develop Team
 * Brief:  httpd
 *
 * Copyright (c) 2020 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
#include "tkc/iostream.h"
#include "tkc/time_now.h"
#include "base/main_loop.h"
#include "tkc/socket_helper.h"
#include "tkc/object_default.h"
#include "tkc/event_source_fd.h"
#include "streams/inet/iostream_tcp.h"

#include "httpd/httpd.h"
#include "httpd/http_route.h"
#include "httpd/http_parser.h"
#include "httpd/http_connection.h"

httpd_t* httpd_create_ex(event_source_manager_t* esm, int port, int max_client) {
  httpd_t* httpd = NULL;
  return_value_if_fail(esm != NULL, NULL);
  return_value_if_fail(port > 0, NULL);
  return_value_if_fail(max_client > 0, NULL);

  httpd = TKMEM_ZALLOC(httpd_t);
  return_value_if_fail(httpd != NULL, NULL);

  httpd->sock = -1;
  httpd->esm = esm;
  httpd->port = port;
  httpd->max_client = max_client;

  return httpd;
}

ret_t httpd_set_routes(httpd_t* httpd, const http_route_entry_t* routes, uint32_t routes_nr) {
  return_value_if_fail(httpd != NULL && routes != NULL && routes_nr > 0, RET_BAD_PARAMS);

  httpd->routes = routes;
  httpd->routes_nr = routes_nr;

  return RET_OK;
}

typedef struct _my_parser_t {
  http_parser parser;

  bool_t completed;
  tk_iostream_t* io;

  char* url;
  char* etag;
  char* query;
  char* content_type;
  int content_length;

  str_t body;
  char key[TK_NAME_LEN + 1];

  httpd_t* httpd;
} my_parser_t;

static ret_t httpd_on_data(event_source_t* source);

static int httpd_on_request_uri(http_parser* parser, const char* p, size_t len) {
  my_parser_t* my = (my_parser_t*)parser;
  my->url = tk_strndup(p, len);

  return 0;
}

static int httpd_on_header_field(http_parser* parser, const char* p, size_t len) {
  my_parser_t* my = (my_parser_t*)parser;

  tk_strncpy(my->key, p, len);

  return 0;
}

static int httpd_on_header_value(http_parser* parser, const char* p, size_t len) {
  my_parser_t* my = (my_parser_t*)parser;

  if (tk_str_ieq(my->key, "Content-Type")) {
    TKMEM_FREE(my->content_type);
    my->content_type = tk_strndup(p, len);
  } else if (tk_str_ieq(my->key, "Content-Length")) {
    my->content_length = tk_atoi(p);
  } else if (tk_str_ieq(my->key, "If-None-Match")) {
    my->etag = tk_strndup(p, len);
  }

  return 0;
}

static int httpd_on_body(http_parser* parser, const char* p, size_t len) {
  my_parser_t* my = (my_parser_t*)parser;
  str_t* str = &(my->body);

  ENSURE(str_append_with_len(str, p, len) == RET_OK);

  return 0;
}

static int httpd_on_message_complete(http_parser* parser) {
  my_parser_t* my = (my_parser_t*)parser;
  str_t* body = &(my->body);
  int method = parser->method;
  http_connection_t* c = NULL;

  log_debug("%s:%s\n%s\n", http_method_str(parser->method), my->url, my->body.str);

  c = http_connection_create(my->io, method, my->url, body->str, body->size);
  assert(c != NULL);
  if (c != NULL) {
    c->etag = my->etag;
    http_route_dispatch(my->httpd->routes, my->httpd->routes_nr, c);
    http_connection_destroy(c);
  }

  my->completed = TRUE;

  return 0;
}

static int httpd_on_message_begin(http_parser* parser) {
  return 0;
}

static const http_parser_settings s_parser_settings = {
    .on_message_begin = httpd_on_message_begin,
    .on_url = httpd_on_request_uri,
    .on_header_field = httpd_on_header_field,
    .on_header_value = httpd_on_header_value,
    .on_body = httpd_on_body,
    .on_message_complete = httpd_on_message_complete};

static http_parser* httpd_parser_create(httpd_t* httpd, int sock) {
  my_parser_t* my = NULL;
  http_parser* parser = NULL;
  my = TKMEM_ZALLOC(my_parser_t);
  return_value_if_fail(my != NULL, NULL);

  my->httpd = httpd;
  my->io = tk_iostream_tcp_create(sock);
  str_init(&(my->body), 100);

  parser = (http_parser*)my;
  http_parser_init(parser, HTTP_REQUEST);

  return parser;
}

static ret_t httpd_parser_destroy(http_parser* parser) {
  my_parser_t* my = (my_parser_t*)parser;

  TKMEM_FREE(my->url);
  TKMEM_FREE(my->query);
  OBJECT_UNREF(my->io);
  str_reset(&(my->body));
  TKMEM_FREE(my->etag);
  TKMEM_FREE(my->content_type);
  memset(parser, 0x00, sizeof(my_parser_t));

  TKMEM_FREE(parser);

  return RET_OK;
}

static ret_t httpd_on_data(event_source_t* source) {
  int ret = 0;
  char buff[2 * 1024];
  event_source_fd_t* event_source_fd = (event_source_fd_t*)source;
  my_parser_t* my = (my_parser_t*)(event_source_fd->ctx);
  tk_istream_t* in = tk_iostream_get_istream(my->io);

  ret = tk_istream_read(in, buff, sizeof(buff) - 1);
  log_debug("client data:ret=%d\n", ret);

  if (ret > 0) {
    buff[ret] = '\0';
    http_parser_execute((http_parser*)my, &s_parser_settings, buff, ret);

    if (my->completed) {
      httpd_parser_destroy((http_parser*)my);
      return RET_REMOVE;
    } else {
      return RET_OK;
    }
  } else {
    httpd_parser_destroy((http_parser*)my);
    return RET_REMOVE;
  }
}

static ret_t httpd_on_client(event_source_t* source) {
  event_source_fd_t* event_source_fd = (event_source_fd_t*)source;
  httpd_t* httpd = (httpd_t*)(event_source_fd->ctx);
  int fd = event_source_get_fd(source);
  int sock = tcp_accept(fd);

  if (sock >= 0) {
    http_parser* parser = httpd_parser_create(httpd, sock);
    log_debug("client connected:%d\n", sock);
    if (parser != NULL) {
      event_source_t* client_source = event_source_fd_create(sock, httpd_on_data, parser);
      event_source_manager_add(httpd->esm, client_source);
      OBJECT_UNREF(client_source);
    } else {
      log_debug("oom! disconnected:%d\n", sock);
      socket_close(sock);
    }
  } else {
    log_debug("error disconnected:%d\n", sock);
    socket_close(sock);
  }
  return RET_OK;
}

ret_t httpd_start(httpd_t* httpd) {
  int sock = -1;
  event_source_t* source = NULL;
  return_value_if_fail(httpd != NULL && httpd->sock < 0, RET_BAD_PARAMS);

  sock = tcp_listen(httpd->port);
  return_value_if_fail(sock >= 0, RET_BAD_PARAMS);

  httpd->sock = sock;
  source = event_source_fd_create(sock, httpd_on_client, httpd);
  return_value_if_fail(source != NULL, RET_OOM);

  log_debug("listen on %d\n", httpd->port);
  event_source_manager_add(httpd->esm, source);
  OBJECT_UNREF(source);

  return RET_OK;
}

ret_t httpd_destroy(httpd_t* httpd) {
  return_value_if_fail(httpd != NULL, RET_BAD_PARAMS);

  if (httpd->sock >= 0) {
    socket_close(httpd->sock);
    httpd->sock = -1;
  }

  TKMEM_FREE(httpd);

  return RET_OK;
}
