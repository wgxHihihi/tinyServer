#include "myhttp.h"
#include <errno.h>
#include <cstring>
#include <assert.h>
#include <sys/sendfile.h>

#define READ_BUF_SIZE 2000

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
    char read_buf[READ_BUF_SIZE] = "";
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

bool myhttp::mywrite()
{
    if (m_flag) //动态请求返回填充体
    {
        int ret_head = send(clientfd, req_head_buf, strlen(req_head_buf), 0);
        int ret_body = send(clientfd, body, strlen(body), 0);
        return ret_head > 0 && ret_body > 0;
    }
    else
    {
        int fd = open(filename, O_RDONLY);
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

void myhttp::doit()
{
}
