#include "myhttp.h"
#include <errno.h>
#include <cstring>
#include <assert.h>
#include <sys/sendfile.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>

myhttp::myhttp()
{
}

myhttp::~myhttp()
{
}

void myhttp::init(int ep_fd, int client_fd)
{
    epfd = ep_fd;
    clientfd = client_fd;
    read_count = 0;
    m_flag = false;
}

bool myhttp::myread()
{
    bzero(&read_buf, sizeof(read_buf));
    while (true)
    {
        int ret = recv(clientfd, read_buf + read_count, READ_BUF_SIZE - read_count, 0);
        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        }
        else if (ret == 0)
        {
            return false;
        }
        read_count += ret;
    }
    strcpy(post_buf, read_buf);
    return true;
}

bool myhttp::mywrite() //发送消息
{
    if (m_flag) //动态请求返回填充体
    {
        int ret_head = send(clientfd, req_head_buf, strlen(req_head_buf), 0);
        int ret_body = send(clientfd, body, strlen(body), 0);
        return ret_head > 0 && ret_body > 0;
    }
    else
    {
        int fd = open(_filename, O_RDONLY);
        assert(fd != -1);
        do
        {
            int ret_head = write(clientfd, req_head_buf, strlen(req_head_buf));
            if (ret_head < 0)
                break;

            int ret_file = sendfile(clientfd, fd, NULL, filesize); //零拷贝，在两个文件标识符间传输文件，读fd（必须是文件），写入clientfd（必须为socket）
            if (ret_file < 0)
                break;

            close(fd);
            return true;
        } while (false);
        close(fd);
        return false;
    }
}

void myhttp::close_connect()
{
    epoll_del(epfd, clientfd);
    close(clientfd);
    clientfd = -1;
}

bool myhttp::get_line(int &index, int &len)
{
    for (; index < len; ++index)
    {
        char c = read_buf[index];
        if (c == '\r' && index + 1 < len && read_buf[index + 1] == '\n') //检测到"\r\n"则取读了一行，将其置'\0'截断字符串
        {
            read_buf[index++] = '\0';
            read_buf[index++] = '\0';
            return true;
        }
    }
    return false;
}

myhttp::HTTP_CODE myhttp::req_parse(char *l_line)
{
    char *line = l_line;
    int len = strlen(line);
    int start_index[3] = {0, 0, 0};
    int i = 0;
    int j = 1;
    while (i < len)
    {
        if (line[i] == ' ')
        {
            line[i++] = '\0';
            start_index[j++] = i;
        }
        i++;
    }
    _method = line + start_index[0];
    if (_method != "GET" && _method != "POST")
        return BAD_REQUESTION;

    _url = line + start_index[1];
    if (_url.empty() || _url[0] != '/')
        return BAD_REQUESTION;

    _protocol = line + start_index[2];
    if (_protocol != "HTTP/1.1")
        return BAD_REQUESTION;

    return NO_REQUESTION;
}

myhttp::HTTP_CODE myhttp::head_parse(char *l_line)
{
    if (l_line[0] == '\0')
    {
        return GET_REQUESTION;
    }
    else
    {
        char *p = l_line;
        std::string key, value;
        while (*p != ':')
            p++;
        *p++ = '\0'; //截断关键字
        key = l_line;

        while (*p == ' ') //删除空格；
            p++;
        value = p;

        _headers[key] = value;
    }
    return NO_REQUESTION;
}

myhttp::HTTP_CODE myhttp::exe_get() //处理get请求
{
    char path[] = PATH;
    int index = 0;
    std::cout << _url << std::endl;
    if ((index = _url.find("?")) != std::string::npos) //动态响应
    {
        _argv = _url.substr(index + 1);
        strcpy(_filename, _url.substr(0, index).c_str());
        return DYNAMIC_FILE;
    }
    else
    {
        strcpy(_filename, path);
        if (_url == "/")
            _url = "/sum.html";
        strcat(_filename, _url.c_str());
        struct stat file_state;
        if (stat(_filename, &file_state) < 0)
            return NOT_FOUND;
        if (!(file_state.st_mode & S_IROTH))
            return FORBIDDEN_REQUESTION;
        if (S_ISDIR(file_state.st_mode)) // is dir?
            return BAD_REQUESTION;

        filesize = file_state.st_size;

        return FILE_REQUESTION;
    }
}

myhttp::HTTP_CODE myhttp::exe_post() //处理post请求
{
    char path[] = PATH;
    strcpy(_filename, path);
    strcat(_filename, _url.c_str());

    if (_headers.count("Content-Length"))
    {
        int argv_len = stoi(_headers["Content-Length"]);
        _argv = post_buf + (strlen(post_buf) - argv_len);
        if (_filename[0] != '\0' && !_argv.empty())
            return POST_FILE;
        else
            return BAD_REQUESTION;
    }
    else
        return BAD_REQUESTION;
}

myhttp::HTTP_CODE myhttp::parse()
{
    PARSE_STATUS state = REQUESTION;
    HTTP_CODE code;
    int read_index = 0;
    int test_index = 0, len = strlen(read_buf);
    while (get_line(test_index, len))
    {
        char *l_line = read_buf + read_index;
        read_index = test_index; //标记下一行起点
        int ret = 0;             //解析结果
        switch (state)
        {
        case REQUESTION:

            std::cout << "requestion\n";
            ret = req_parse(l_line);
            if (ret == BAD_REQUESTION)
            {
                std::cout << "ret==BAD_REQUESTION";
                return BAD_REQUESTION;
            }
            state = HEAD; //处理完请求行继续处理 头部 和 消息体
            break;
        case HEAD:
            ret = head_parse(l_line);
            if (ret == GET_REQUESTION) // head_parse解析到文件末尾了才为true
            {
                if (_method == "GET")
                    return exe_get();
                else if (_method == "POST")
                    return exe_post();
                else
                    return BAD_REQUESTION;
            }
            break;
        default:
            return INTERNAL_ERROR;
        }
    }
    return NO_REQUESTION;
}

void myhttp::bad_response()
{
    /*400 response */
    _url = "/bad.html";
    bzero(_filename, sizeof(_filename));
    strcpy(_filename, PATH);
    strcat(_filename, _url.c_str());
    struct stat my_file;
    if (stat(_filename, &my_file) < 0)
        std::cout << "bad_response.html not found !";
    filesize = my_file.st_size;
    bzero(req_head_buf, sizeof(req_head_buf));
    sprintf(req_head_buf, "HTTP/1.1 400 BAD_REQUESTION\r\nConnection: close\r\ncontent-length:%d\r\n\r\n", filesize);
}

void myhttp::forbidden_response()
{
    _url = "/forbidden.html";
    bzero(_filename, sizeof(_filename));
    strcpy(_filename, PATH);
    strcat(_filename, _url.c_str());
    struct stat my_file;
    if (stat(_filename, &my_file) < 0)
        std::cout << "forbidden_response.html not found !";
    filesize = my_file.st_size;
    bzero(req_head_buf, sizeof(req_head_buf));
    sprintf(req_head_buf, "HTTP/1.1 403 FORBIDDEN\r\nConnection: close\r\ncontent-length:%d\r\n\r\n", filesize);
}

void myhttp::notfound_response()
{
    _url = "/not_found.html";
    bzero(_filename, sizeof(_filename));
    strcpy(_filename, PATH);
    strcat(_filename, _url.c_str());
    struct stat my_file;
    if (stat(_filename, &my_file) < 0)
        std::cout << "notfound_response.html not found !";
    filesize = my_file.st_size;
    bzero(req_head_buf, sizeof(req_head_buf));
    sprintf(req_head_buf, "HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\ncontent-length:%d\r\n\r\n", filesize);
}

void myhttp::file_response()
{
    m_flag = false;
    bzero(req_head_buf, sizeof(req_head_buf));
    sprintf(req_head_buf, "HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length:%d\r\n\r\n", filesize);
}

void myhttp::post_response()
{
    if (fork() == 0)
    {
        dup2(clientfd, STDOUT_FILENO);
        execl(_filename, _argv.c_str(), NULL);
    }
    wait(NULL);
}

void myhttp::dynamic_response()
{
    m_flag = true;
    int num[2];
    int res = 0;
    bzero(req_head_buf, sizeof(req_head_buf));
    sscanf(_argv.c_str(), "a=%d&b=%d", &num[0], &num[1]);
    std::cout << num[0] << ", " << num[1] << std::endl;
    std::cout << _filename << std::endl;
    if (strcmp(_filename, "/add") == 0)
    {
        res = num[0] + num[1];

        sprintf(body, "<html><body>\r\n<p>%d + %d = %d </p><hr>\r\n</body></html>\r\n", num[0], num[1], res);
        sprintf(req_head_buf, "HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length: %zd\r\n\r\n", strlen(body));
    }
    else if (strcmp(_filename, "/multiplication") == 0)
    {
        std::cout << "\t\t\t\tmultiplication\n\n";
        sprintf(body, "<html><body>\r\n<p>%d * %d = %d </p><hr>\r\n</body></html>\r\n", num[0], num[1], num[0] * num[1]);
        sprintf(req_head_buf, "HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length: %zd\r\n\r\n", strlen(body));
    }
}

/*
NO_REQUESTION,        //请求不完整，需要客户继续输入
GET_REQUESTION,       //获得并且解析了一个正确的HTTP请求
BAD_REQUESTION,       // HTTP请求语法不正确
FORBIDDEN_REQUESTION, //访问资源的权限有问题
FILE_REQUESTION,      // GET方法资源请求
INTERNAL_ERROR,       //服务器自身问题
NOT_FOUND,            //请求的资源文件不存在
DYNAMIC_FILE,         //动态请求
POST_FILE
*/
void myhttp::response() //应答请求
{
    HTTP_CODE code = parse();
    switch (code)
    {
    case NO_REQUESTION:
        std::cout << "Incomplete requestion\n";
        epoll_mod(epfd, clientfd, EPOLLIN);
        break;
    case BAD_REQUESTION: // 400
        std::cout << "bad requestion\n";
        bad_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    case FORBIDDEN_REQUESTION: // 403
        std::cout << "forbiddenr requestion\n";
        forbidden_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    case NOT_FOUND: // 404
        std::cout << "not found\n";
        notfound_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    case FILE_REQUESTION:
        std::cout << "file requestion\n";
        file_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    case DYNAMIC_FILE:
        std::cout << "dynamic file\n";
        dynamic_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    case POST_FILE:
        std::cout << "post requestion\n";
        post_response();
        epoll_mod(epfd, clientfd, EPOLLOUT);
        break;
    default:
        close_connect();
    }
}
