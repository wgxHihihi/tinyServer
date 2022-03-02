#ifndef _MYLOCK_H_
#define _MYLOCK_H_
#include <semaphore.h>

//信号量，用于协调线程池的调度
class sem
{
private:
    sem_t m_sem;

public:
    sem();
    ~sem();
    bool wait(); //等待信号量
    bool post(); //增加信号量
};

//互斥锁，用于任务队列互斥访问；
class mylocker
{
private:
    pthread_mutex_t m_mutex; //互斥锁；
public:
    mylocker();
    ~mylocker();
    bool lock();
    bool unlock();
};

//条件变量，不满足条件时阻塞线程并释放线程持有的互斥锁，条件一旦满足则被唤醒，并重新获得互斥锁，向下执行
class mycond
{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;

public:
    mycond();
    ~mycond();
    bool wait();
    bool signal(); //发通知
};

#endif