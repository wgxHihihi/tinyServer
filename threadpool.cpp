#include "threadpool.h"

/*创建线程池*/
template <class T>
threadpool<T>::threadpool()
{
    max_thread = 8;
    max_job = 1000;
    m_stop = false;
    m_threadpool = new pthread_t[max_thread]; //可换成智能指针
    if (!m_threadpool)
    {
        throw std::exception();
    }
    //创建线程
    for (int i = 0; i < max_thread; ++i)
    {
        if (pthread_create(m_threadpool + i, NULL, worker, this) != 0)
        {
            delete[] m_threadpool;
            throw std::exception();
        }
        if (pthread_detach(m_threadpool[i])) //将状态改为unjoinable状态，确保资源的释放
        {
            delete[] m_threadpool;
            throw std::exception();
        }
    }
}
/*线程池析构*/
template <class T>
threadpool<T>::~threadpool()
{
    delete[] m_threadpool;
    m_stop = true;
}
/*添加任务*/
template <class T>
bool threadpool<T>::addjob(T *request)
{
    //多线程访问共享资源需要上锁
    m_queueLoker.lock();
    if (m_workQueue.size() > max_job)
    {
        m_queueLoker.unlock();
        return false;
    }
    m_workQueue.emplace_back(request);
    m_queueLoker.unlock();
    m_queueState.post(); //信号量加一；
    return true;
}
/*pthread格式的任务函数*/
template <class T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}
/*线程执行任务*/
template <class T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        m_queueState.wait(); //信号量减一，为0时阻塞等待
        m_queueLoker.lock();
        if (m_workQueue.empty())
        {
            m_queueLoker.unlock();
            continue;
        }
        T *request = m_workQueue.front();
        m_workQueue.pop_front();
        m_queueLoker.unlock();
        if (!request)
        {
            continue;
        }
        request->response();
    }
}