#include "mySocket.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

/*Definitions of the members of class MySocket*/
mySocket::mySocket() {}
mySocket::~mySocket() {}
bool mySocket::init_socket()
{
    /************创建socket*************/
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < -1)
    {
        snprintf(err_msg, sizeof(err_msg), "create socket error: %s (srrno:%d)\n", strerror(errno), errno); //将错误信息写到 err_msg中
        return false;
    }
    return true;
}

char *mySocket::get_err_msg()
{
    return err_msg;
}

/*Definitions of the members of class MyServerSocket*/
bool myServerSocket::create_server(unsigned int port)
{
    /**
     * @brief 设置socket并实施 bind，listen 操作。
     *
     */
    memset(&server_addr, 0, sizeof(server_addr)); //结构体清零初始化
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // host to unsigned long
    server_addr.sin_port = htons(port);              //将port转化为网络字节顺序
    // 获取socket状态标志
    flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK); //将socket设置为非阻塞方式，便于I/O多路复用
    //设置超时
    struct timeval timeout = {3, 0}; //时间结构体，分别表示秒和毫秒
    if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "set send timeout failed: %s(errno: %d)\n", strerror(errno), errno); //设置发送超时
        return false;
    }
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "set receive timeout failed: %s(errno: %d)\n", strerror(errno), errno); //设置发送超时
        return false;
    }
    //设置端口复用，避免端口占用，供server binding
    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /************绑定端口*************/
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        snprintf(err_msg, sizeof(err_msg), "binding error: %s(errno: %d)\n", strerror(errno), errno); //设置发送超时
        return false;
    }
    /************监听端口*************/
    if (listen(sock_fd, 10) == -1)
    {
        snprintf(err_msg, sizeof(err_msg), "listening error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    return true;
}

bool myServerSocket::accept_client(int &client_sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_len; //和int 一样，为socket体系中的变量，用于存储client_addr的长度
    /************接受连接*************/
    client_sock = accept(sock_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd <= 0)
    {
        snprintf(err_msg, sizeof(err_msg), "accept error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }
    client_fd = client_sock; //确认accept成功后再赋值
    return true;
}
/*need to do*/
bool recv_data(char *buffer, int buffer_len, int &rec_len);
bool send_data(const char *data, int len);
void close_client(int client_sock);
void server_close();

bool client_connect(const int *ip, unsigned port);
bool recv_data(char *buffer, int buffer_len, int &rec_len);
bool send_data(const char *data, int len);
void client_close();