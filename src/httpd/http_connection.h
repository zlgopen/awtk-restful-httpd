/**
 * File:   http_connection.h
 * Author: AWTK Develop Team
 * Brief:  http_connection
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

#ifndef TK_HTTP_CONNECTION_H
#define TK_HTTP_CONNECTION_H

#include "tkc/iostream.h"
#include "conf_io/conf_json.h"

BEGIN_C_DECLS
/** 
 * @class http_connection_t
 * 
 * http connection. 
 */
typedef struct _http_connection_t {
  /**
   * @property {tk_iostream_t*} io
   * sock 
   */
  tk_iostream_t* io;

  /**
   * @property {int32_t} method
   * http request method
   */
  int32_t method;

  /**
   * @property {const char*} etag
   * etag
   */
  const char* etag;
  /**
   * @property {const char*} url
   * url
   */
  const char* url;
  /**
   * @property {object_t*} args
   * args from url
   */
  object_t* args;

  /**
   * @property {conf_doc_t*} req
   * request data
   */
  conf_doc_t* req;

  /**
   * @property {conf_doc_t*} resp
   * response data
   */
  conf_doc_t* resp;
} http_connection_t;

/**
 * @method http_connection_create
 * 创建connection对象。
 * 
 * @param {tk_iostream_t*} io iostream
 * @param {int32_t} method http method
 * @param {const char*} url url 
 * @param {const char*} body body
 * @param {uint32_t} body_size body_size
 * 
 * @return {http_connection_t*} 返回connection对象。
 */
http_connection_t* http_connection_create(tk_iostream_t* io, int32_t method, const char* url,
                                          const char* body, uint32_t body_size);

/**
 * @method http_connection_send
 * 
 * 向客户端发送数据。
 * 
 * @param {http_connection_t*} c connection对象。
 * @param {int} status 响应码。
 * @param {const char*} extra_header 额外的头信息。
 * @param {const void*} body body数据。
 * @param {uint32_t} size  body数据长度。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_connection_send(http_connection_t* c, int status, const char* extra_header,
                           const void* body, uint32_t size);

/**
 * @method http_connection_send_file
 * 
 * 向客户端发送文件。
 * 
 * @param {http_connection_t*} c connection对象。
 * @param {const char*} filename 文件名。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_connection_send_file(http_connection_t* c, const char* filename);

/**
 * @method http_connection_send_ok
 * 
 * 向客户端发送成功的响应。
 * 
 * @param {http_connection_t*} c connection对象。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_connection_send_ok(http_connection_t* c);
/**
 * @method http_connection_send_fail
 * 
 * 向客户端发送失败的响应。
 * 
 * @param {http_connection_t*} c connection对象。
 * @param {int32_t} status_code http status_code
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_connection_send_fail(http_connection_t* c, int32_t status_code);

/**
 * @method http_connection_destroy
 * 
 * 销毁connection对象。
 * 
 * @param {http_connection_t*} c connection对象。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_connection_destroy(http_connection_t* c);

END_C_DECLS

#endif /*TK_HTTP_CONNECTION_H*/
