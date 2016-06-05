#ifndef __semaphore_h__
#define __semaphore_h__

#include "system_defines.h"

#ifndef WIN32
#include <semaphore.h>
#endif

/*  Semaphore - a general synchronization mechanism */
class Semaphore
{
public:
    /*  Constructor. Sets the semaphore's value to 0.  */
    Semaphore( void );

    /*  Constructor. Sets the semaphore to the given value.
        @param aValue Value to initialize semaphore with.
    */
    Semaphore(int value);
    ~Semaphore( void );

    /*  Unlocks the semaphore.
        If the semaphore value resulting from this operation  is
        positive, then no threads were blocked waiting for the
        semaphore to become unlocked; the semaphore value is simply
        incremented.
        If the value of the semaphore resulting from this operation
        is 0, then one of the threads blocked waiting for the semaphore
        will be allowed to return successfully from its call to wait(). 
        This method is reentrant with respect to signals
        and may be invoked from a signal-catching function.
    */
    void post( void );

    /*  Locks the semaphore.
        If the semaphore value is currently zero, then the calling thread
        will not return from this method until it either locks
        the semaphore or the call is interrupted by a signal.
    */
    void wait( void );

private:
    Semaphore( const Semaphore& );
    Semaphore& operator=( const Semaphore& );

#ifndef WIN32
    sem_t m_sem;
#else 
    HANDLE m_sem;
#endif
};

#endif /* __semaphore_h__ */
