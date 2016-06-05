#ifndef WIN32
#include <sys/time.h>
#endif

#include "condition.h"
#include "system_exception.h"
#include "mutex.h"

Condition::Condition()
{
#ifndef WIN32
    if( 0 != pthread_cond_init( &m_cond, NULL ) )
    {
        throw system_exception("pthread_cond_init", errno);
    }
#else 
  
    /* Initialize the count to 0 */
    m_cond.waitersCount = 0;

    InitializeCriticalSection(&m_cond.waitersCountLock);
  
    /* Create an auto-reset event */
    m_cond.events_[pthread_cond_t::SIGNAL] = CreateEvent( NULL,  /* no security */
                                                          FALSE, /* auto-reset event */
                                                          FALSE, /* non-signaled initially */
                                                          NULL );/* unnamed */

    /* Create a manual-reset event */
    m_cond.events_[pthread_cond_t::BROADCAST] = CreateEvent( NULL,
                                                             TRUE,  /* manual-reset */
                                                             FALSE,
                                                             NULL );
#endif
}

Condition::~Condition()
{
#ifndef WIN32
    pthread_cond_destroy(&m_cond );
#else 
    ::CloseHandle( m_cond.events_[pthread_cond_t::SIGNAL] );
    ::CloseHandle( m_cond.events_[pthread_cond_t::BROADCAST] );
    ::DeleteCriticalSection( &m_cond.waitersCountLock );
#endif
}

void Condition::broadcast()
{
#ifndef WIN32
    if( 0 != pthread_cond_broadcast( &m_cond ) )
    {
        throw system_exception("pthread_cond_broadcast", errno);
    }
#else 
    /* Avoid race conditions */
    EnterCriticalSection( &m_cond.waitersCountLock );
    bool haveWaiters = (m_cond.waitersCount > 0);
    LeaveCriticalSection( &m_cond.waitersCountLock );

    if (haveWaiters)
        SetEvent( m_cond.events_[pthread_cond_t::BROADCAST] );
#endif
}

void Condition::signal()
{
#ifndef WIN32
    if( 0 != pthread_cond_signal(&m_cond) )
    {
        throw system_exception("pthread_cond_signal", errno);
    }
#else
 
    /* Avoid race conditions */
    EnterCriticalSection( &m_cond.waitersCountLock );
    int have_waiters = m_cond.waitersCount > 0;
    LeaveCriticalSection( &m_cond.waitersCountLock );

    if (have_waiters)
        SetEvent( m_cond.events_[pthread_cond_t::SIGNAL] );
#endif
}

void Condition::wait( Mutex* pMutex )
{
#ifndef WIN32
    if( 0 != pthread_cond_wait( &m_cond, &pMutex->m_mutex ) )
    {
        throw system_exception("pthread_cond_wait", errno);
    }
#else
  
    /* Avoid race conditions */
    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount++;
    LeaveCriticalSection( &m_cond.waitersCountLock );

    /*  It's ok to release the <external_mutex> here since Win32
        manual-reset events maintain state when used with
        <SetEvent>.  This avoids the "lost wakeup" bug...   */
    pMutex->unlock();

    /*  Wait for either event to become signaled due to <pthread_cond_signal>
        being called or <pthread_cond_broadcast> being called */
    int result = WaitForMultipleObjects( 2, m_cond.events_, FALSE, INFINITE );

    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount--;
    bool isLastWaiter = ( (result == WAIT_OBJECT_0 + pthread_cond_t::BROADCAST)
                          && (m_cond.waitersCount == 0) );
    LeaveCriticalSection( &m_cond.waitersCountLock );

    /* Some thread called <pthread_cond_broadcast> */
    if( isLastWaiter )
    {
        /*  We're the last waiter to be notified or to stop waiting, so
            reset the manual event. */
        ResetEvent( m_cond.events_[pthread_cond_t::BROADCAST] );
    }

    /*  Reacquire the <external_mutex> */
    pMutex->lock();

#endif
}

bool Condition::timed_wait( Mutex* pMutex, u64 aTimeout )
{
#ifndef WIN32
    struct timeval tValue;                                                  
    if( 0 != gettimeofday(&tValue,NULL) )
    {
        system_exception("gettimeofday", errno);                       
    }    

    struct timespec absTime;
    absTime.tv_sec = tValue.tv_sec + aTimeout / 1000;
    long nanoSeconds = tValue.tv_usec*1000 + (aTimeout % 1000) * 1000000;
    absTime.tv_sec += nanoSeconds / 1000000000;   
    absTime.tv_nsec = nanoSeconds % 1000000000;          

    int res = pthread_cond_timedwait( &m_cond, &pMutex->m_mutex, &absTime );
    if( 0 != res )
    {
        if(ETIMEDOUT == res)
        {
            return false;
        }
        else
        {
            throw system_exception("pthread_cond_timedwait", errno);
        }
    } 
#else
    /* Avoid race conditions */
    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount++;
    LeaveCriticalSection( &m_cond.waitersCountLock );

    /*  It's ok to release the <external_mutex> here since Win32
        manual-reset events maintain state when used with
        <SetEvent>.  This avoids the "lost wakeup" bug... */
    pMutex->unlock();

    /*  Wait for either event to become signaled due to <pthread_cond_signal>
        being called or <pthread_cond_broadcast> being called */
    int result = WaitForMultipleObjects( 2, m_cond.events_, FALSE, static_cast<DWORD>(aTimeout) );

    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount--;
    bool isLastWaiter = ( (result == WAIT_OBJECT_0 + pthread_cond_t::BROADCAST)
                          && (m_cond.waitersCount == 0) );
    LeaveCriticalSection( &m_cond.waitersCountLock );

    /* Some thread called <pthread_cond_broadcast> */
    if( isLastWaiter )
    {
        /*  We're the last waiter to be notified or to stop waiting, so
            reset the manual event. */
        ResetEvent( m_cond.events_[pthread_cond_t::BROADCAST] ); 
    }

    /* Reacquire the <external_mutex> */
    pMutex->lock();  

    if( WAIT_TIMEOUT == result )
    {
        return false;
    }
#endif
    return true;
}

#ifdef WIN32
bool Condition::timed_waitUntilEvent( Mutex* pMutex, HANDLE event )
{
   /* Avoid race conditions */
    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount++;
    LeaveCriticalSection( &m_cond.waitersCountLock );

    /*  It's ok to release the <external_mutex> here since Win32
        manual-reset events maintain state when used with
        <SetEvent>.  This avoids the "lost wakeup" bug... */
    pMutex->unlock();

    HANDLE events[3];
    events[0] = m_cond.events_[0];
    events[1] = m_cond.events_[1];
    events[2] = event;

    /* Wait for either event to become signaled due to <pthread_cond_signal>
        being called or <pthread_cond_broadcast> being called */
    int result = WaitForMultipleObjects( 3, events, FALSE, INFINITE );

    EnterCriticalSection( &m_cond.waitersCountLock );
    m_cond.waitersCount--;
    bool isLastWaiter = ( (result == WAIT_OBJECT_0 + pthread_cond_t::BROADCAST)
                          && (m_cond.waitersCount == 0) );
    LeaveCriticalSection (&m_cond.waitersCountLock);

    /* Some thread called <pthread_cond_broadcast> */
    if( isLastWaiter )
    {
        /* We're the last waiter to be notified or to stop waiting, so
           reset the manual event. */
        ResetEvent( m_cond.events_[pthread_cond_t::BROADCAST] ); 
    }

    /* Reacquire the <external_mutex> */
    pMutex->lock();  

    /* if event was fired - returns false */
    if( WAIT_OBJECT_0 + 2 == result )
    {
        return false;
    }
    return true;
}
#endif
