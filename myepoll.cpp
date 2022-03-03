#include "myepoll.h"
#include <stdio.h>

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
int epoll_init()
{
    int epoll_fd = epoll_create(LISTENQ + 1);
    if (epoll_fd == -1)
        return -1;
    // events = (struct epoll_event*)malloc(sizeof(struct epoll_event) *
    // MAXEVENTS);
    return epoll_fd;
}
void epoll_add(int epfd, int fd, bool is_oneshot)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (is_oneshot)
        ev.events |= EPOLLONESHOT;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
}

void epoll_mod(int epfd, int fd, epoll_event ev)
{
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

void epoll_del(int epfd, int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
}

int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout)
{
    int ret_count = epoll_wait(epoll_fd, events, max_events, timeout);
    if (ret_count < 0)
    {
        perror("epoll wait error");
    }
    return ret_count;
}