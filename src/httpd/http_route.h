/**
 * File:   http_route.h
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

#ifndef TK_HTTP_ROUTE_H
#define TK_HTTP_ROUTE_H

#include "http_connection.h"

BEGIN_C_DECLS

/*http处理函数*/
typedef ret_t (*http_handler_t)(http_connection_t* c);

typedef struct _http_route_entry_t {
  /*请求方法: HTTP_GET/HTTP_POST/HTTP_DELETE/HTTP_PUT */
  int method;
  /**
   * 匹配模式
   *> 路径中以':'开头的视为参数。
   *> 如/wd/:sesssion/element/:id，其中:sesssion和:id为参数，可以匹配任意字符串。
   **/
  const char* pattern;
  /*处理函数*/
  http_handler_t handle;
} http_route_entry_t;

/**
 * @class http_route_t 
 * @annotation ["fake"]
 * 
 * 负责HTTP路由管理。
 */

/**
 * @method http_route_dispatch
 * @annotation ["static"]
 * @param {const http_route_entry_t*} routes 路由表。
 * @param {uint32_t} nr 路由表的数目。
 * @param {http_connection_t*} c HTTP连接对象。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_route_dispatch(const http_route_entry_t* routes, uint32_t nr, http_connection_t* c);

/**
 * @method http_route_match
 * 检查模式是否与url匹配。
 * 
 *> 路径中以':'开头的视为参数。
 *> 如/wd/:sesssion/element/:id，其中:sesssion和:id为参数，可以匹配任意字符串。
 * 
 * @annotation ["static"]
 * @param {const char*} pattern 模式。
 * @param {const char*} url URL。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
bool_t http_route_match(const char* pattern, const char* url);

/**
 * @method http_route_parse_args
 * 根据模式解析URL中的参考，并放入args对象。
 *
 * > 路径中以':'开头的视为参数。
 *> 如/wd/:sesssion/element/:id，其中:sesssion和:id为参数，可以匹配任意字符串。
 * @annotation ["static"]
 * 
 * @param {const char*} pattern 模式。
 * @param {const char*} url URL。
 * @param {object_t*} args 用于保存解析的参数。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_route_parse_args(const char* pattern, const char* url, object_t* args);

/**
 * @method http_route_handle_static_file
 * 处理静态文件。
 *
 * @annotation ["static"]
 * 
 * @param {http_connection_t*} c HTTP连接对象。
 * @param {const char*} web_root 静态文件的根目录。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t http_route_handle_static_file(http_connection_t* c, const char* web_root);

END_C_DECLS

#endif /*TK_HTTP_ROUTE_H*/
