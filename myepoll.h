#ifndef _MYEPOLL_H_
#define _MYEPOLL_H_
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>

const int MAXEVENT = 1000;
const int LISTENQ = 5;
int epoll_init();
int setnonblocking(int fd);
void epoll_add(int epfd, int fd, bool is_oneshot);
void epoll_mod(int epfd, int fd, int ev);
void epoll_del(int epfd, int fd);
int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);

#endif