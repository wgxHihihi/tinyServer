#ifndef _MYHTTP_H_
#define _MYHTTP_H_

class myhttp
{
private:
    int a;

public:
    myhttp(/* args */);
    ~myhttp();
    void init(int epfd, int client_fd);
    bool myread();
    bool mywrite();
    void close_connect();
};

myhttp::myhttp(/* args */)
{
}

myhttp::~myhttp()
{
}

#endif