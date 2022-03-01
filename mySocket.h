#ifndef MYSOCKET_H_
#define MYSOCKET_H_
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

// socket基类
class mySocket
{
protected: //公有继承子类可以访问但子类的对象不能访问；
    int sock_fd;
    struct sockaddr_in server_addr; // addr 和 port分开存储 便于赋值操作，实际做参数是转换为 const sockaddr* 类型使用
    char err_msg[256];

public:
    mySocket();
    ~mySocket();
    bool init_socket();
    char *get_err_msg();
};
//服务器socket
class myServerSocket : public mySocket
{
private:
    int recvbyres, res, flags;
    int client_fd;

public:
    bool create_server(unsigned int prot);
    bool accept_client(int &client_sock);
    bool recv_data(char *buffer, int buffer_len, int &rec_len);
    bool send_data(const char *data, int len);
    void close_client(int client_sock);
    void server_close();
};
//客户端socket
class myclientSocket : public mySocket
{
public:
    bool client_connect(const int *ip, unsigned port);
    bool recv_data(char *buffer, int buffer_len, int &rec_len);
    bool send_data(const char *data, int len);
    void client_close();
};

#endif
