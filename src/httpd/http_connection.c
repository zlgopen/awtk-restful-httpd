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

#include "tkc/fs.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/object_default.h"
#include "httpd/http_connection.h"

static const char* http_status_text(int code) {
  switch (code) {
    case 100:
      return "Continue";
    case 101:
      return "Switching Protocol";
    case 102:
      return "Processing (WebDAV)";
    case 103:
      return "Early Hints";
    case 200:
      return "OK";
    case 201:
      return "Created";
    case 202:
      return "Accepted";
    case 203:
      return "Non-Authoritative Information";
    case 204:
      return "No Content";
    case 205:
      return "Reset Content";
    case 206:
      return "Partial Content";
    case 207:
      return "Multi-Status (WebDAV)";
    case 208:
      return "Already Reported (WebDAV)";
    case 226:
      return "IM Used (HTTP Delta encoding)";
    case 300:
      return "Multiple Choice";
    case 301:
      return "Moved Permanently";
    case 302:
      return "Found";
    case 303:
      return "See Other";
    case 304:
      return "Not Modified";
    case 305:
      return "Use Proxy";
    case 306:
      return "unused";
    case 307:
      return "Temporary Redirect";
    case 308:
      return "Permanent Redirect";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 402:
      return "Payment Required";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    case 406:
      return "Not Acceptable";
    case 407:
      return "Proxy Authentication Required";
    case 408:
      return "Request Timeout";
    case 409:
      return "Conflict";
    case 410:
      return "Gone";
    case 411:
      return "Length Required";
    case 412:
      return "Precondition Failed";
    case 413:
      return "Payload Too Large";
    case 414:
      return "URI Too Long";
    case 415:
      return "Unsupported Media Type";
    case 416:
      return "Range Not Satisfiable";
    case 417:
      return "Expectation Failed";
    case 418:
      return "I'm a teapot";
    case 421:
      return "Misdirected Request";
    case 422:
      return "Unprocessable Entity (WebDAV)";
    case 423:
      return "Locked (WebDAV)";
    case 424:
      return "Failed Dependency (WebDAV)";
    case 425:
      return "Too Early";
    case 426:
      return "Upgrade Required";
    case 428:
      return "Precondition Required";
    case 429:
      return "Too Many Requests";
    case 431:
      return "Request Header Fields Too Large";
    case 451:
      return "Unavailable For Legal Reasons";
    case 500:
      return "Internal Server Error";
    case 501:
      return "Not Implemented";
    case 502:
      return "Bad Gateway";
    case 503:
      return "Service Unavailable";
    case 504:
      return "Gateway Timeout";
    case 505:
      return "HTTP Version Not Supported";
    case 506:
      return "Variant Also Negotiates";
    case 507:
      return "Insufficient Storage (WebDAV)";
    case 508:
      return "Loop Detected (WebDAV)";
    case 510:
      return "Not Extended";
    case 511:
      return "Network Authentication Required";
    default:
      return "Unknown";
  }
}

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

ret_t http_connection_send(http_connection_t* c, int status, const char* extra_header,
                           const void* body, uint32_t size) {
  int ret = 0;
  char header[512];
  tk_ostream_t* out = tk_iostream_get_ostream(c->io);

  memset(header, 0x00, sizeof(header));
  ret =
      tk_snprintf(header, sizeof(header) - 1,
                  "HTTP/1.1 %d %s\r\n"
                  "%s"
                  "Content-Length: %u\r\n\r\n",
                  status, http_status_text(status), extra_header != NULL ? extra_header : "", size);

  ret = tk_ostream_write_len(out, header, strlen(header), 1000);
  if (size > 0) {
    ret = tk_ostream_write_len(out, body, size, 1000);
  }

  if (size > 1000) {
    log_debug("%d:%s\n", ret, header);
  } else {
    log_debug("%d:%s\n", ret, (char*)body);
  }

  return RET_OK;
}

static const char* get_content_type_of_filename(const char* filename) {
  const char* ext = strrchr(filename, '.');
  return_value_if_fail(ext != NULL, "");

  if (tk_str_eq(ext, ".json")) {
    return "application/json; charset=utf-8";
  } else if (tk_str_eq(ext, ".js")) {
    return "application/json; charset=utf-8";
  } else if (tk_str_eq(ext, ".png")) {
    return "application/json; charset=utf-8";
  } else if (tk_str_eq(ext, ".jpg")) {
    return "application/json; charset=utf-8";
  } else if (tk_str_eq(ext, ".txt")) {
    return "application/json; charset=utf-8";
  } else if (tk_str_eq(ext, ".c")) {
    return "text/plain; charset=utf-8";
  } else {
    return "text/plain; charset=utf-8";
  }
}

ret_t http_connection_send_file(http_connection_t* c, const char* filename) {
  int ret = 0;
  char etag[64];
  int status = 200;
  char* buffer = NULL;
  fs_stat_info_t info;
  uint32_t buffer_size = 4000;
  int32_t size = file_get_size(filename);
  tk_ostream_t* out = tk_iostream_get_ostream(c->io);
  fs_file_t* fp = fs_open_file(os_fs(), filename, "rb");
  const char* status_text = http_status_text(status);
  const char* content_type = get_content_type_of_filename(filename);
  return_value_if_fail(fp != NULL, RET_BAD_PARAMS);

  buffer = TKMEM_ALLOC(buffer_size);
  if (buffer == NULL) {
    fs_file_close(fp);
    fp = NULL;
    return_value_if_fail(buffer != NULL, RET_BAD_PARAMS);
  }

  fs_file_stat(fp, &info);
  tk_snprintf(etag, sizeof(etag), "W/\"%llu-%llu\"", info.mtime, info.size);

  memset(buffer, 0x00, buffer_size);
  if(tk_str_eq(etag, c->etag)) {
    ret = tk_snprintf(buffer, buffer_size - 1,
                      "HTTP/1.1 304 Not Modified\r\n"
                      "Content-Length: 0\r\n\r\n");
    log_debug("%s\n", buffer);
    ret = tk_ostream_write_len(out, buffer, strlen(buffer), 1000);
  } else {
    ret = tk_snprintf(buffer, buffer_size - 1,
                      "HTTP/1.1 %d %s\r\n"
                      "ETag: %s\r\n"
                      "Content-Type: %s\r\n"
                      "Cache-Control: public, max-age=604800, immutable\r\n"
                      "Content-Length: %u\r\n\r\n",
                      status, status_text, etag, content_type, size);

    log_debug("%s\n", buffer);
    ret = tk_ostream_write_len(out, buffer, strlen(buffer), 1000);
    while ((ret = fs_file_read(fp, buffer, buffer_size)) > 0) {
      tk_ostream_write_len(out, buffer, ret, 1000);
    }
  }
  fs_file_close(fp);

  return RET_OK;
}

ret_t http_connection_send_ok(http_connection_t* c) {
  str_t str_body;
  ret_t ret = RET_OK;
  str_t* body = &(str_body);
  const char* header =
      "Content-Type: application/json; charset=utf-8\r\nCache-Control: no-cache\r\n";
  str_init(body, 512);
  conf_doc_save_json(c->resp, body);
  ret = http_connection_send(c, 200, header, body->str, body->size);
  str_reset(body);

  return ret;
}

ret_t http_connection_send_fail(http_connection_t* c, int32_t status_code) {
  int ret = 0;
  char header[256];
  tk_ostream_t* out = tk_iostream_get_ostream(c->io);
  return_value_if_fail(c != NULL, RET_BAD_PARAMS);

  tk_snprintf(header, sizeof(header),
              "HTTP/1.1 %d Server Error\r\n"
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
