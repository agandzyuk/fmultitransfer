#include "mutex.h"

#include <stdio.h>
#include "system_defines.h"
#include "system_exception.h"

#ifndef WIN32
pthread_once_t g_onceMutex = PTHREAD_ONCE_INIT;

pthread_mutexattr_t Mutex::s_mutexAttr;


void onceMutexRoutine()
{
    i32 ret = pthread_mutexattr_init(&Mutex::s_mutexAttr);
    if( 0 != ret )
    {
        throw system_exception("pthread_mutexattr_init", ret);
    }
#ifndef _LINUX
    ret = pthread_mutexattr_settype(&Mutex::s_mutexAttr, PTHREAD_MUTEX_NORMAL);
    if( 0 != ret )
    {
        throw system_exception("pthread_mutexattr_settype", ret);
    }
#else
    ret = pthread_mutexattr_settype(&Mutex::s_mutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if( 0 != ret )
    {
        throw system_exception("pthread_mutexattr_settype", ret);
    }
#endif /* #ifndef _LINUX */
}

#endif /* #ifndef WIN32 */

Mutex::Mutex()
#ifndef WIN32
    : m_Owner(0)
#endif /* #ifndef WIN32 */
{
#ifndef WIN32
    pthread_once(&g_onceMutex, ::onceMutexRoutine);

    i32 ret = pthread_mutex_init(&m_mutex, &Mutex::s_mutexAttr);
    if( 0 != ret )
    {
        throw system_exception("pthread_mutex_init", ret);
    }
#else /* WIN32 */
    ::InitializeCriticalSection(&m_mutex);
#endif
}

Mutex::~Mutex()
{
#ifndef WIN32
    /*i32 rv = */pthread_mutex_destroy(&m_mutex);
    // assert( (EBUSY != rv) && "An attempt was detected to destroy the  object  referenced  by  mutex while it is locked or referenced (for example, while being used in a pthread_cond_wait(3THR) or pthread_cond_timedwait(3THR)) by another thread.");
    // assert( 0 == rv);
  
#else 
    DeleteCriticalSection(&m_mutex);
#endif
}

void Mutex::lock()
{
#ifndef WIN32 
    /* Try and lock the mutex  */
    i32 res = pthread_mutex_trylock(&m_mutex );
    if( 0 != res )
    {
        if( EBUSY != res )
        {
            throw system_exception("pthread_mutex_lock", res);
        }
        else
        {
        /* The mutex is already locked, if someone else has it locked, we block until it becomes available */
            if( pthread_self() != m_Owner )
            {
                res = pthread_mutex_lock(&m_mutex );
                if( 0 != res )
                {
                    throw system_exception("pthread_mutex_lock", res);
                }
            }
        }
    }
    m_Owner = pthread_self();
#else /* WIN32 */
    EnterCriticalSection(&m_mutex);
#endif
}

void Mutex::unlock()
{
#ifndef WIN32
    m_Owner = 0;
    i32 ret = pthread_mutex_unlock(&m_mutex);
    if( 0 != ret )
    {
        throw system_exception("pthread_mutex_unlock", ret);
    }
#else /* WIN32 */
    LeaveCriticalSection(&m_mutex);
#endif
}

bool Mutex::tryLock()
{
#ifdef WIN32
    return FALSE != TryEnterCriticalSection( &m_mutex );
#else
    i32 res = pthread_mutex_trylock(&m_mutex );
    switch(res)
    {
    case 0:
        return true;
    case EBUSY:
        return false;
    default:
        throw system_exception("pthread_mutex_lock", res);
    };
    return false;
#endif
}
