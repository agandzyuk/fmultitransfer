#ifndef __condition_h__
#define __condition_h__

#include "common_types.h"
#include "system_defines.h"

#ifndef WIN32
#include <time.h>
#include <pthread.h>
#endif

class Mutex;
/*  Condition class provides communication, the ability to wait for some shared
    resource to reach some desired state, or to signal that it has reached some
    state in which another thread may be interested. Each condition variable is
    closely associated with a mutex that protects the state of the resource.
    @note A conditional variable wait always returns with the mutex locked.
    @warning Conditional variables are for signaling, not for mutual exclusion.
    @note Conditional variables and thier predicates are "linked" - for best results, 
    treat them that way!
*/
class Condition
{
public:
    /*  Constructor.
        @throw system_exception
    */
    Condition( void );
    ~Condition( void );

    /*  Broadcasts condition, waking all current waiters.
        @note Use when more than one waiter may respond to predicate change or
        if any waiting thread may not be able to respond.
        @throw system_exception
    */
    void broadcast( void );

    /*  Signals condition, waking one waiting thread.
        @note Use when any waiter can respond, and only one need respond. All waiters are equal.
        @throw system_exception
    */
    void signal( void );

    /*  Waits on condition, until awakened by a signal or broadcast.
        The current thread must own specified mutex.
        @throw system_exception
   
        @note The function shall block on a condition variable. It shall be
        called with mutex locked by the calling thread or undefined behavior
        results. Upon successful return, the mutex shall have been locked and
        shall be owned by the calling thread.
    */
    void wait( Mutex* pMutex );

    /*  Waits on condition, until awaked by a signal or broadcast, or until 
        a specified amount of time has elapsed.
        If awaked by a signal or broadcast returns true, otherwise false.
        The current thread must own specified mutex. 
        @note The function shall block on a condition variable. It shall be
        called with mutex locked by the calling thread or undefined behavior
        results. Upon successful return, the mutex shall have been locked and
        shall be owned by the calling thread.
        @param aTimeout the maximum time to wait in milliseconds.   
        @throw system_exception
    */
    bool timed_wait( Mutex* pMutex, u64 timeout );

#ifdef _WIN32
    /*  Waits on condition, until awaked by a signal or broadcast, or until 
        a specified event fired.
        If awaked by a signal or broadcast returns true, otherwise false.
        The current thread must own specified mutex. 
        @note The function shall block on a condition variable. It shall be
        called with mutex locked by the calling thread or undefined behavior
        results. Upon successful return, the mutex shall have been locked and
        shall be owned by the calling thread.
        @param event - event descriptor to stop waiting.   
        @throw system_exception
    */
    bool timed_waitUntilEvent( Mutex* pMutex, HANDLE event );
#endif
  
private:
    Condition( const Condition& );
    Condition& operator=( const Condition& );

#ifdef _WIN32
    typedef struct pthread_cond_t
    {
        unsigned int waitersCount; /* Count of the number of waiters */
        /* Serialize access to <waitersCount> */
        CRITICAL_SECTION waitersCountLock;
  
        enum 
        {
            SIGNAL     = 0,
            BROADCAST  = 1,
            MAX_EVENTS = 2
        };

        HANDLE events_[MAX_EVENTS];
        /* Signal and broadcast event HANDLEs */
  
    } pthread_cond_t;
#endif
    pthread_cond_t m_cond;
};

#endif /* __condition_h__ */
