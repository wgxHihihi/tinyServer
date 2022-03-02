#include "mylock.h"
#include <exception>
#include <pthread.h>
/*******sem difinitions*******/
sem::sem()
{
    if (sem_init(&m_sem, 0, 0) != 0)
    {
        throw std::exception();
    }
}

sem::~sem()
{
    sem_destroy(&m_sem);
}

bool sem::wait()
{
    return sem_wait(&m_sem) == 0;
}

bool sem::post()
{
    return sem_post(&m_sem) == 0;
}

/*******mylocker difinitions*******/
mylocker ::mylocker(/* args */)
{
    if (pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}
mylocker::~mylocker()
{
    pthread_mutex_destroy(&m_mutex);
}
bool mylocker::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}
bool mylocker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

/*******mycond difinitions*******/
mycond::mycond(/* args */)
{
    if (pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
    if (pthread_cond_init(&m_cond, NULL) != 0)
    {
        throw std::exception();
    }
}

mycond::~mycond()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

bool mycond::wait()
{
    int ret = 0;
    pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&m_cond, &m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool mycond::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}