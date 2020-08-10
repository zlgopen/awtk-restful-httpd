# awtk-restful-httpd

在嵌入式应用程序中，有时需要提供一个 WEB 服务，用于对系统进行远程配置和管理。awtk-restful-httpd 实现了一个 RESTful HTTP 服务框架，可以帮助开发者快速实现 RESTful API 风格的 WEB 服务。主要特色有：

* 简单。
* 易用。
* 方便嵌入到 AWTK 应用程序。

## 准备

1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

2. 获取 awtk-restful-httpd 并编译
```
git clone https://github.com/zlgopen/awtk-restful-httpd.git
cd awtk-restful-httpd; scons
```

## 运行

> 考虑到 github 的下载速度，生成的临时资源没放到 git 中，需要自己生成资源：

```
./scripts/update_res.py all
```

```
./bin/demo
```

## 示例

* 1. 定义路由表

```c

static ret_t my_httpd_on_status(http_connection_t* c) {
  return RET_OK;
}

static ret_t my_httpd_on_element_action(http_connection_t* c) {
  return RET_OK;
}

static const http_route_entry_t s_my_httpd_routes[] = {
  {HTTP_GET, "/status", my_httpd_on_status},
  {HTTP_GET, "element/:element/:action", my_httpd_on_element_action}
};
```

* 2. 启动服务

```c
ret_t my_httpd_start(httpd_t* httpd) {
  return_value_if_fail(httpd != NULL, RET_BAD_PARAMS);

  httpd_set_routes(httpd, s_my_httpd_routes, ARRAY_SIZE(s_my_httpd_routes));
  
  return httpd_start(httpd);
}
```

> 完整示例请参考：demos
