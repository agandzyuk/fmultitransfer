#ifndef __mutex_h__
#define __mutex_h__

#include "system_defines.h"
#include "guard.h"

#ifndef WIN32
#include <pthread.h>
#endif


/*
#ifndef WIN32
    extern "C"{ void onceMutexRoutine(); }
#endif
*/

/*
 * Mutex - provides synchronization, the ability to control how threads share
 * resources. You use mutexes to prevent multiple threads from modifying
 * shared data at the same time, and to ensure that a thread can read
 * consistent values for a set of resources (for example, memory) that may be
 * modified by other threads.
 */
class Mutex
{
#ifndef WIN32
    friend class Condition;
    friend void onceMutexRoutine();
#endif

    friend class Guard<Mutex>;

public:
    /* Constructor. */
    Mutex(); 

    /* Destructor. */
    virtual ~Mutex();

    /* 
     * Locks the mutex. If the mutex is currently locked, the calling thread is
     * blocked until mutex is unlocked. On return, the thread owns the mutex until
     * it calls Mutex::unlock().
     */
    void lock();

    bool tryLock();

    /*
     * Unlocks the mutex. The mutex becomes unowned. If any threads are waiting
     * for the mutex, one is awakened.
     */
    void unlock();

#ifndef WIN32
    /* Describes the attributes of a thread mutex. It should be considered 
     * an opaque record, the names of the fields can change anytime.
     */
    static pthread_mutexattr_t s_mutexAttr;
#endif

private:

#ifndef WIN32
    pthread_mutex_t  m_mutex;
    volatile pthread_t m_Owner;
#else
    CRITICAL_SECTION m_mutex;
#endif

    Mutex(const Mutex& aM);
    Mutex& operator=(const Mutex& aRhs);      
};

typedef Guard<Mutex> MGuard;

#endif /*__mutex_h__ */
