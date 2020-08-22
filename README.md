# awtk-restful-httpd

## 1. 介绍

在嵌入式应用程序中，有时需要提供一个 WEB 服务，用于对系统进行远程配置和管理。

awtk-restful-httpd 实现了一个 RESTful HTTP 服务框架，可以帮助开发者快速实现 RESTful API 风格的 WEB 服务。主要特色有：

* 小巧。约 3000 行代码。
* 简单。注册自己路由表处理函数即可。
* 易用。内置路由解析功能，并提供简单易用的 JSON 读写 API。
* 内存开销低。正常 REST API 内存需求小于 2K，静态文件内存需求小于 6K。
* 方便嵌入到 AWTK 应用程序。无缝集成到 AWTK 的主循环，无需另外开辟线程。

## 2 准备

### 2.1 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

### 2.2 获取 awtk-restful-httpd 并编译

```
git clone https://github.com/zlgopen/awtk-restful-httpd.git
cd awtk-restful-httpd
```

* 生成资源

```
python ./scripts/update_res.py all
```

> 或者通过 designer 生成资源

* 编译 PC 版本

```
scons
```

* 编译 LINUX FB 版本

```
scons LINUX_FB=true
```

> 完整编译选项请参考 [编译选项](https://github.com/zlgopen/awtk-widget-generator/blob/master/docs/build_options.md)

## 3. 运行

```
./bin/demo
```

## 4. 示例

### 4.1 定义路由表

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

### 4.2 静态文件

内置提供了静态文件的处理，支持静态文件处理相当容易。

```c
static ret_t my_httpd_on_get_file(http_connection_t* c) {
  return http_route_handle_static_file(c, "static");
}
```

> "static"在这里表示静态文件所在的目录。

路由匹配规则用“*”表示缺省路由，放到最后一项，用来处理静态文件。

```c
{HTTP_GET, "*", my_httpd_on_get_file},
```

> 完整示例请参考：demos/app_main.c

### 4.3 静态文件传输优化

有些大的文件，特别是文本文件，可以压缩之后传输，能大大提供传输效率。为了避免运行时压缩带来的内存和 CPU 开销，可以预先压缩文件，并以.gz 文件命名。

比如请求 static/js/awtk\_asm.wasm 文件时，如果存在 static/js/awtk\_asm.wasm.gz，那么就返回 static/js/awtk\_asm.wasm.gz 给客户端，并设置 Content-Encoding，浏览器会自动解压。

```
Content-Encoding: gzip
```

### 4.4 启动服务

```c
ret_t my_httpd_start(httpd_t* httpd) {
  return_value_if_fail(httpd != NULL, RET_BAD_PARAMS);

  httpd_set_routes(httpd, s_my_httpd_routes, ARRAY_SIZE(s_my_httpd_routes));
  
  return httpd_start(httpd);
}
```

> [awtk-ui-automation](https://github.com/zlgopen/awtk-ui-automation) 是 awtk-restful-httpd 的第一个应用，可以作为示例代码参考。
