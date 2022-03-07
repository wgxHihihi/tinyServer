# tinyServer

web server in Cpp. simplified demo

## 简介

1. 非阻塞socket+epoll（ET模式）
2. reactor +practor 模式：
   1. 主线程负责epoll监听并接受新的连接请求;
   2. 读取EPOLLIN状态的socket并将任务添加到threadpool;
   3. 向EPOLLOUT的socket写入数据；
3. 实现了GET 方法和POST方法；
4. 实现了400，403，404错误码的响应；

## 调试方法

1. 终端中 cd 至 build/ 文件夹
2. 执行 `cmake ../`
3. 执行 `make` 进行编译，CMakeLists.txt 中有链接指令，执行make后会在build文件夹生成可执行文件 TinyServer
4. 执行 `./TinyServer` 运行服务器程序
5. 浏览器输入`localhost:8888/sum.html`或`localhost:8888` 执行get方法访问

## 注意

请将 myhttp.h 文件中的PATH 定义为相应的用户目录。
