#ifndef _MYHTTP_H_
#define _MYHTTP_H_
#include "myepoll.h"
#include <string>
#include <map>
#define BUFFER_SIZE 1000
#define READ_BUF_SIZE 2000

class myhttp
{
private:
    int m_flag; //动态请求标志，true表示是动态请求
    char post_buf[BUFFER_SIZE];
    char read_buf[READ_BUF_SIZE];
    char req_head_buf[BUFFER_SIZE];
    char body[BUFFER_SIZE];
    char _filename[256]; //静态请求文件目录
    int filesize;        //待传输文件的字节数
    /********请求数据格式******/
    std::string _method;                         //请求方法
    std::string _url;                            //请求路径
    std::string _protocol;                       // HTTP协议
    std::string _argv;                           //消息体
    std::string _host;                           //主机位置
    std::map<std::string, std::string> _headers; //其他头部信息

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

    void close_connect();
    //获取一行HTTP请求，插入\0截断read_buf
    bool get_line(int &start_index, int &len);
    //解析请求行
    HTTP_CODE req_parse(char *l_line);
    //解析头部
    HTTP_CODE head_parse(char *l_line);
    //处理get请求
    HTTP_CODE exe_get();
    //处理post请求
    HTTP_CODE exe_post();
    //解析HTTP请求并返回HTTP请求类型；
    HTTP_CODE parse();

    /****响应程序****/
    /*写入回复数据，由mywrite传输*/

    /*400错误*/
    void bad_response();
    /*404错误*/
    void notfound_response();
    /*403错误*/
    void forbidden_response();
    /*get返回访问文件*/
    void file_response();
    /*post用子进程去执行post请求*/
    void post_response();
    /*动态页面，post用动态页面返回结果*/
    void dynamic_response();

    /****执行响应****/
    void response();
};

#endif