#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include <list>
#include "mylock.h"
/*封装threadpool，底层为pthread*/

template <class T>
class threadpool
{
private:
    int max_thread;             //线程池最大线程数；
    int max_job;                //工作队列最大线程数；
    pthread_t *m_threadpool;    //线程池数组
    std::list<T *> m_workQueue; //请求队列

    mylocker m_queueLoker; //请求对列读取保护锁；
    sem m_queueState;      //是否有任务需要处理的信号量
    bool m_stop;           //是否结束线程
public:
    threadpool(int max_th = 8, int max_j = 10);
    ~threadpool();
    bool addjob(T *request);

private:
    static void *worker(void *arg);
    void run();
};

#endif