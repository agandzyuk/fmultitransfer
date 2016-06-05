#ifndef __thread_h__
#define __thread_h__

#include <string>
#include "system_exception.h"
#include "mutex.h"

#ifndef WIN32
#   include <pthread.h>
typedef pthread_t ThreadId;
#else
typedef ui32 ThreadId;
#endif /* WIN32 */

/* extern "C"{ void onceInitRoutine(); } */
extern "C"{ void exitRoutine(void *) ; }

/*
* A thread of execution in a program. 
*/
class Thread 
{
private:
    std::string m_name;

    Mutex lock_;
    ThreadId m_id;

#ifdef WIN32
    HANDLE m_handle;
    bool setTranslator_;
#endif /* WIN32 */

public:
    /*
    * Constructor. 
    * When a new thread is created, it does not begin immediate execution.
    * @param setExTranslator - valid for the Windows platform only. When true SEH 
    * exceptions will be translated into the Utils::Exception.
    *
    * @throw system_exception
    *
    * @see start()    
    */
    explicit Thread( bool setExTranslator = true );

    /*
    * Constructor. 
    When a new thread is created, it does not begin immediate execution.
    * @param setExTranslator - valid for the Windows platform only. When true SEH 
    * exceptions will be translated into the Utils::Exception.
    *
    * @throw system_exception
    *
    * @see start() 
    */
    explicit Thread( const s8 *pName, bool setExTranslator = true );
    explicit Thread( std::string aName, bool setExTranslator = true );

    /* 
    * Destructor. 
    */
    virtual ~Thread();

    /*
    * Causes this thread to begin execution. 
    *
    * @throw system_exception
    */
    void start();

    /* 
    * Returns the thread's name. 
    */
    std::string const& getName() const { return m_name; }

    /*
    * Waits for this thread to terminate.
    *
    * @throw system_exception
    */
    void join() ;

    /*
    * Causes the currently executing thread object to temporarily pause 
    * and allow other threads to execute.
    *
    * @throw system_exception
    */
    static void yield();

    /*
    * Returns the thread ID of the calling thread.
    */
    static ThreadId self();

    /*
    @return True if thread IDs equals, false otherwise.
    */
    bool equals( ThreadId thrId );

    /*
    Requests that thread to be canceled. 
    *
    * Cancellation is asynchronous. Use join() to wait for termination of
    * thread if necessary.
    * You should never use cancellation unless you really want the target thread
    * to go away. This is a termination mechanism.
    *
    * @throw system_exception
    *
    * @see "cancellation (3THR)"
    */
    void cancel();

    /*
    Causes the currently executing thread to usleep for the specified number
    of mls.
    *
    * It is a cancellation point.
    *
    @return true if the method was interrupted by a signal, false otherwise.
    *
    * @throw system_exception
    */
    static bool sleep( u32 aMilis );

    /*
    * Returns thread's id.
    */
    ThreadId getId() const { return m_id; }

    /*
    * Gives name to the current thread
    * @param name New name
    */
    static void setName( s8 const* name );

    /*
    * Interrupts this thread.    
    *
    * @throw system_exception
    */
    void interrupt();

#ifndef WIN32
    static const i32 s_INTERRUPT_SIGNAL;
#endif

protected:
    /*
    * Thread's start function.
    * Subclasses of Thread should override this method.
    */
    virtual void run() = 0;

    typedef enum{DISABLED, DEFERRED, ASYNCHRONOUS} CancellationType;

    /*
    * Sets thread's cancellation mode. 
    *
    * @throw system_exception
    */
    void setCancelMode(CancellationType aMode);

private:
    /*
    * @throw system_exception
    */
    void init( bool setExTranslator );
    void cleanup();

    Thread( const Thread& aT );
    Thread& operator=( const Thread& aRhs );

    static void launcher( Thread* pThr );
    static void launcher2( Thread* pThr );
    static void exitRoutine( void* );

#ifndef WIN32  
    static void onceInitRoutine();
    static void* startHook(void* apArg);
#else 
    static ui32 __stdcall startHook( void* apArg );
#endif /* WIN32  */
};

#ifdef WIN32  
    /* Attach SEH translator function to translate them into Exception */
    extern void attachExceptionTranslator();
#endif

#endif /* __thread_h__ */
