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

#ifndef TK_HTTPD_H
#define TK_HTTPD_H

#include "tkc/types_def.h"
#include "httpd/http_route.h"
#include "httpd/http_parser.h"
#include "httpd/http_connection.h"

BEGIN_C_DECLS

/**
 * @class httpd_t
 * http服务。
 */
typedef struct _httpd_t {
  void* user_data;

  /*private*/
  int sock;
  int port;
  int max_client;
  int clients_nr;
  uint32_t routes_nr;
  const http_route_entry_t* routes;
  event_source_manager_t* esm;
} httpd_t;

/**
 * @method httpd_create_ex
 * 创建httpd对象。
 * @param {event_source_manager_t*} esm 事件管理器。
 * @param {int} port 监听的端口。
 * @param {int} max_client 最大客户数。
 * 
 * @return {httpd_t*} 返回httpd对象。
 */
httpd_t* httpd_create_ex(event_source_manager_t* esm, int port, int max_client);

#define httpd_create(port, max_client) httpd_create_ex(main_loop_get_event_source_manager(main_loop()), port, max_client)

/**
 * @method httpd_set_routes
 * 设置路由表。
 * @param {httpd_t*} httpd httpd对象。
 * @param {http_route_entry_t*} routes 路由条目。
 * @param {uint32_t} routes_nr 路由条目数。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t httpd_set_routes(httpd_t* httpd, const http_route_entry_t* routes, uint32_t routes_nr);

/**
 * @method httpd_start
 * 启动服务。
 * @param {httpd_t*} httpd httpd对象。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t httpd_start(httpd_t* httpd);

/**
 * @method httpd_destroy
 * 销毁httpd对象。
 * @param {httpd_t*} httpd httpd对象。
 * 
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t httpd_destroy(httpd_t* httpd);

END_C_DECLS

#endif /*TK_HTTPD_H*/
