#ifndef _MYHTTP_H_
#define _MYHTTP_H_
#include "myepoll.h"
#define BUFFER_SIZE 1000
class myhttp
{
private:
    int m_flag; //动态请求标志，true表示是动态请求
    char post_buf[BUFFER_SIZE];
    char req_head_buf[BUFFER_SIZE];
    char body[BUFFER_SIZE];
    char filename[250]; //静态请求文件目录
    int filesize;       //待传输文件的字节数

public:
    enum HTTP_CODE
    {
        NO_REQUESTION,        //请求不完整，需要客户继续输入
        GET_REQUESTION,       //获得并且解析了一个正确的HTTP请求
        BAD_REQUESTION,       // HTTP请求语法不正确
        FORBIDDEN_REQUESTION, //访问资源的权限有问题
        FILE_REQUESTION,      // GET方法资源请求
        INTERNAL_ERROR,       //服务器自身问题
        NOT_FOUND,            //请求的资源文件不存在
        DYNAMIC_FILE,         //动态请求
        POST_FILE             //以POST方式请求的HTTP请求
    };
    enum PARSE_STATUS
    {
        HEAD,      //解析头部信息
        REQUESTION //解析请求行
    };

public:
    int epfd;
    int clientfd;
    int read_count;

    myhttp();
    ~myhttp();
    void init(int ep_fd, int client_fd);
    bool myread();
    bool mywrite();
    void doit();
    void close_connect();
};

#endif