#include "awtk.h"
#include "httpd/httpd.h"

static httpd_t* s_httpd = NULL;

static ret_t my_httpd_on_file(http_connection_t* c) {
  http_connection_send_file(c, "demos/app_main.c");
  return RET_STOP;
}

static ret_t my_httpd_on_status(http_connection_t* c) {
  value_t v;
  conf_doc_set(c->resp, "status", value_set_int(&v, 0));
  conf_doc_set_str(c->resp, "message", "hello awtk");

  return RET_OK;
}

static ret_t my_httpd_on_element_action(http_connection_t* c) {
  value_t v;
  const char* element = object_get_prop_str(c->args, "element");
  const char* action = object_get_prop_str(c->args, "action");

  log_debug("element:%s action=%s\n", element, action); 

  conf_doc_set(c->resp, "status", value_set_int(&v, 0));
  conf_doc_set(c->resp, "value.element", value_set_str(&v, element));
  conf_doc_set(c->resp, "value.action", value_set_str(&v, action));

  return RET_OK;
}

static const http_route_entry_t s_my_httpd_routes[] = {
  {HTTP_GET, "/status", my_httpd_on_status},
  {HTTP_GET, "/file", my_httpd_on_file},
  {HTTP_GET, "element/:element/:action", my_httpd_on_element_action}
};

ret_t my_httpd_start(httpd_t* httpd) {
  return_value_if_fail(httpd != NULL, RET_BAD_PARAMS);

  httpd_set_routes(httpd, s_my_httpd_routes, ARRAY_SIZE(s_my_httpd_routes));
  
  return httpd_start(httpd);
}

static ret_t on_close_clicked(void* ctx, event_t* e) {
  tk_quit();
  return RET_OK;
}

static ret_t main_window_create(void) {
  widget_t* win = window_open("main");
  widget_child_on(win, "close", EVT_CLICK, on_close_clicked, NULL);

  return RET_OK;
}

ret_t application_init(void) {
  socket_init();
  main_window_create();
  s_httpd = httpd_create(8000, 1);

  return my_httpd_start(s_httpd);
}

ret_t application_exit(void) {
  httpd_destroy(s_httpd);
  socket_deinit();
  
  return RET_OK;
}

#include "../res/assets.inc"
#include "awtk_main.inc"
