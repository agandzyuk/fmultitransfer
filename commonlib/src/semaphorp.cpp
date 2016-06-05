#include "semaphorp.h"
#include "system_exception.h"

using namespace std;

const int MAXIMUM_COUNT = INT_MAX;

Semaphore::Semaphore()
{
#ifndef _WIN32
    if( -1 == sem_init(&m_sem, 0, 0) )
    {
        throw system_exception("sem_init", errno);
    }
#else 
    m_sem = CreateSemaphore( NULL, 0, MAXIMUM_COUNT, NULL );
    if( NULL == m_sem )
    {
        throw system_exception("CreateSemaphore");
    }
#endif
}

Semaphore::Semaphore( int value ) 
{
#ifndef WIN32
    if( -1 == sem_init(&m_sem, 0, value) )
    {
        throw system_exception("sem_init", errno);
    }
#else
    m_sem = CreateSemaphore( NULL, 
                             value, 
                             MAXIMUM_COUNT, 
                             NULL );
    if( NULL == m_sem )
    {
        throw system_exception("CreateSemaphore");
    }
#endif
}

Semaphore::~Semaphore()
{
#ifndef WIN32
    sem_destroy( &m_sem );
#else 
    BOOL ret = CloseHandle( m_sem );
    assert( 0 != ret );
#endif
}

void Semaphore::post()
{
#ifndef WIN32
    if( -1 == sem_post( &m_sem ) )
    {
        throw system_exception("sem_post", errno);
    }
#else
    BOOL ret = ReleaseSemaphore( m_sem, 1, NULL );
    if( 0 == ret )
    {
        throw system_exception("ReleaseSemaphore");
    }
#endif
}

void Semaphore::wait() 
{
#ifndef WIN32
    while( 0 != sem_wait( &m_sem ) )
    {
        if( EINTR != errno )
        {
            throw system_exception("sem_wait", errno);
        }
    }
#else 
    DWORD ret = WaitForSingleObject( m_sem, INFINITE );
    if(WAIT_FAILED == ret)
    {
        throw system_exception("WaitForSingleObject");
    }
#endif
}
