#include <iostream>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "threadpool.h"
#include "myepoll.h"
#include "myhttp.h"

int create_bind_listen(int port)
{
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cout << "create socket failed!\n";
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); //结构体清零初始化
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // host to unsigned long
    server_addr.sin_port = htons(port);              //将port转化为网络字节顺序
    //设置端口复用，避免端口占用，供server binding
    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /************绑定端口*************/
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cout << "binding error: %s(errno: %d)\n"; //设置发送超时
        return false;
    }
    /************监听端口*************/
    if (listen(sock_fd, 1024) == -1)
    {
        std::cout << "listening error: %s(errno: %d)\n";
        return -1;
    }
    if (sock_fd == -1)
    {
        close(sock_fd);
        return -1;
    }
    return sock_fd;
}
int acceptConnect(int listenfd)
{
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client_fd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addrlen);
    if (client_fd < 0)
    {
        std::cout << "accept error!\n";
        return -1;
    }
    return client_fd;
}

int main()
{
    threadpool<myhttp> *pool = new threadpool<myhttp>;
    // http服务
    myhttp *users = new myhttp[100];
    assert(users);
    //创建监听socket
    int listenfd = create_bind_listen(8888);
    assert(listenfd > -1);
    //创建epoll
    epoll_event events[MAXEVENT];
    int epfd = epoll_init();
    assert(epfd != -1);
    //添加监听事件
    epoll_add(epfd, listenfd, false);

    while (true)
    {
        int num = my_epoll_wait(epfd, events, MAXEVENT, -1);
        if (num == 0)
            continue;
        for (int i = 0; i < num; ++i)
        {
            int sockfd = events[i].data.fd;
            //监听socket有数据写入，说明有新的连接接入
            if (sockfd == listenfd)
            {
                int client_fd = acceptConnect(listenfd);
                assert(client_fd > -1);
                epoll_add(epfd, client_fd, true);
                std::cout << "client_fd:" << client_fd << "****\n";
                users[client_fd].init(epfd, client_fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                if (users[sockfd].myread())
                {
                    //读取成功加入任务队列
                    pool->addjob(users + sockfd);
                }
                else
                {
                    //读取失败则关闭连接
                    users[sockfd].close_connect();
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (users[sockfd].mywrite())
                {
                    users[sockfd].close_connect();
                }
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //发生异常关闭客户端连接
                users[sockfd].close_connect();
            }
        }
    }
    close(epfd);
    close(listenfd);
    delete[] users;
    delete pool;
    return 0;
}