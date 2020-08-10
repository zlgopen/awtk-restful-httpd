#include "httpd/http_route.h"
#include "tkc/object_default.h"
#include "gtest/gtest.h"

TEST(HttpRoute, match) {
  //ASSERT_EQ(http_route_match("/wd/sesssion", "/wd/sesssion"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion", "/wd/sesssion/1234"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/delete", "/wd/sesssion/1234/delete"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/element", "/wd/sesssion/1234/element"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/element/:id/click", 
        "/wd/sesssion/1234/element/start/click"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/element/:id/:action", 
        "/wd/sesssion/1234/element/start/click"), TRUE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/element/:id/click", 
        "/wd/sesssion/1234/element/start/enumerator"), FALSE);
  ASSERT_EQ(http_route_match("/wd/sesssion/:sesssion/element", 
        "/wd/sesssion/1234/element/start/enumerator"), FALSE);
}

TEST(HttpRoute, parse1) {
  object_t* args = object_default_create();
  http_route_parse_args("/wd/sesssion/:session", "/wd/session/1234", args);
  ASSERT_STREQ(object_get_prop_str(args, "session"), "1234");

  OBJECT_UNREF(args);
}

TEST(HttpRoute, parse2) {
  object_t* args = object_default_create();
  http_route_parse_args("/wd/sesssion/:session/element", "/wd/session/1234/element", args);
  ASSERT_STREQ(object_get_prop_str(args, "session"), "1234");

  OBJECT_UNREF(args);
}

TEST(HttpRoute, parse3) {
  object_t* args = object_default_create();
  http_route_parse_args("/wd/sesssion/:session/element/:element", "/wd/session/1234/element/start", args);
  ASSERT_STREQ(object_get_prop_str(args, "session"), "1234");
  ASSERT_STREQ(object_get_prop_str(args, "element"), "start");

  OBJECT_UNREF(args);
}

TEST(HttpRoute, parse4) {
  object_t* args = object_default_create();
  http_route_parse_args("/wd/sesssion/:session/element/:element/:action", 
      "/wd/session/1234/element/start/click", args);
  ASSERT_STREQ(object_get_prop_str(args, "session"), "1234");
  ASSERT_STREQ(object_get_prop_str(args, "element"), "start");
  ASSERT_STREQ(object_get_prop_str(args, "action"), "click");

  OBJECT_UNREF(args);
}

TEST(HttpRoute, parse5) {
  object_t* args = object_default_create();
  http_route_parse_args("/wd/sesssion/:session/element/:element/:action/", 
      "/wd/session/1234/element/start/click/", args);
  ASSERT_STREQ(object_get_prop_str(args, "session"), "1234");
  ASSERT_STREQ(object_get_prop_str(args, "element"), "start");
  ASSERT_STREQ(object_get_prop_str(args, "action"), "click");

  OBJECT_UNREF(args);
}

