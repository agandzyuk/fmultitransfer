#include "thread.h"

#include <iostream>
#include <string.h>
#include <typeinfo>

#include "system_defines.h"
#include "generic_exception.h"
#include "system_exception.h"
#include "guard.h"

#ifdef WIN32
#   include <process.h>
#   include <eh.h>              /* For _set_se_translator */
#else 
#   include <sys/time.h>
#   include <signal.h>
#endif /* WIN32 */

using namespace std;

#ifdef WIN32
/*****************************************************************
* Windows specific stuffs
******************************************************************/
#ifndef __BCPLUSPLUS__
class CSE 
{
public:
    /* Call this function for each thread. */
    static void MapSEtoCE() { _set_se_translator(TranslateSEtoCE); }
    operator DWORD() { return(m_er.ExceptionCode); }
private:
    CSE(PEXCEPTION_POINTERS pep) {
        m_er      = *pep->ExceptionRecord;
        m_context = *pep->ContextRecord;
    }
    static void _cdecl TranslateSEtoCE(UINT /*dwEC*/,
        PEXCEPTION_POINTERS pep) {
            throw CSE(pep);
    }
private:
    EXCEPTION_RECORD m_er;      /* CPU independent exception information */
    CONTEXT          m_context; /* CPU dependent exception information */
};
#   endif /* __BCPLUSPLUS__ */

static void trans_func( ui32 /*u*/, EXCEPTION_POINTERS* ptr )
{
    if( (NULL == ptr) || (NULL == ptr->ExceptionRecord) )
        return;

    switch( ptr->ExceptionRecord->ExceptionCode )
    {
    case EXCEPTION_ACCESS_VIOLATION:
    assert( false && "Fatal error: Memory access violation exception!" );
        throw Exception("Fatal error: Access violation exception!");
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        throw Exception("Fatal error: Data misaligment!");
    case EXCEPTION_BREAKPOINT:
        break;
    case EXCEPTION_SINGLE_STEP:
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        throw Exception("Fatal error: Array bounds exceeded!");
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        throw Exception("Fatal error: Float divide by zero!");
    case EXCEPTION_FLT_INEXACT_RESULT:
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        throw Exception("Fatal error: Invalid operation with float!");
    case EXCEPTION_FLT_OVERFLOW:
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        throw Exception("Fatal error: Stack overflowed/underflowed after float operation!");
    case EXCEPTION_FLT_UNDERFLOW:
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        throw Exception("Fatal error: Int divided by zero!");
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_PRIV_INSTRUCTION:
        throw Exception("Fatal error: Instruction isn't allowed in current mode!");
    case EXCEPTION_IN_PAGE_ERROR:
        throw Exception("Fatal error: Page isn't present!");
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        throw Exception("Fatal error: Invalid instruction!");
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        throw Exception("Fatal error: Unable to continue execution after previous error!");
    case EXCEPTION_STACK_OVERFLOW:
        throw Exception("Fatal error: Stack overflow!");
    case EXCEPTION_INVALID_DISPOSITION:
        throw Exception("Fatal error: Invalid disposition of the dispatcher!");
    case EXCEPTION_GUARD_PAGE:
        throw Exception("Fatal error: Operation with guard page!");
    case EXCEPTION_INVALID_HANDLE:
        throw Exception("Fatal error: Operation with invalid handle!");
    };
}

void attachExceptionTranslator()
{
    _set_se_translator( trans_func );
}

ui32 __stdcall Thread::startHook( void* apArg ) 
{
    Thread::launcher( reinterpret_cast<Thread*>(apArg) );
    return NULL;
}
#else /* WIN32 */
/*****************************************************************
* Linux specific stuffs
******************************************************************/

const i32 Thread::s_INTERRUPT_SIGNAL = SIGUSR1;
pthread_once_t g_onceControl = PTHREAD_ONCE_INIT;

void* Thread::startHook( void* apArg ) 
{
    Thread::launcher(reinterpret_cast<Thread*>(apArg) );
    return NULL;
}

void interrruptHandler( i32 /*aSigNo*/ )
{
    cerr << "interrruptHandler, id=" << pthread_self() << endl;
}

void dummy()
{}

void Thread::onceInitRoutine()
{
    struct sigaction sigAction;
    sigAction.sa_flags = 0;
    sigAction.sa_handler = &interrruptHandler;
    sigemptyset(&sigAction.sa_mask);
    i32 status = sigaction(Thread::s_INTERRUPT_SIGNAL, &sigAction, NULL);
    if( 0 != status )
    {
        cerr << "Error in initRoutine()\n";
    }
}
#endif /* WIN32 */

Thread::Thread( const s8 *pName, bool setExTranslator ) 
    : m_name(pName), m_id(0)
#ifdef WIN32
    , m_handle(0)
#endif
{
    init(setExTranslator);
}

Thread::Thread( std::string aName, bool setExTranslator ) 
    : m_name(aName), m_id(0)
#ifdef WIN32
    , m_handle(0)
#endif
{
    init(setExTranslator);
}

Thread::Thread( bool setExTranslator ) 
    : m_name(""), m_id(0)
#ifdef WIN32
    , m_handle(0)
#endif
{
    init( setExTranslator );
}

#ifndef WIN32
void Thread::init( bool setExTranslator )
{
    i32 ret = pthread_once( &g_onceControl, setExTranslator ? onceInitRoutine : dummy );
    if( 0 != ret )
    {
        throw system_exception("pthread_once", ret);
    }
}
#else 
void Thread::init( bool setExTranslator )
{
    setTranslator_ = setExTranslator;
}
#endif /* WIN32 */

void Thread::cleanup()
{
    MGuard g(lock_);
#ifdef WIN32
    if(0 != m_handle)
    {
        BOOL ok = CloseHandle(m_handle);
        assert(ok);
        m_handle = 0;
    }
#endif
}

Thread::~Thread()
{
    cleanup();
}

void Thread::launcher2( Thread* pThr )
{
#ifdef WIN32
    __try
    {
#endif /* WIN32 */
        pThr->run();
#ifdef WIN32
    }
    __except( assert("Unhandled exception." && false ), EXCEPTION_CONTINUE_SEARCH )
    {
    }
#endif /* WIN32 */
}

void Thread::launcher( Thread* pThr )
{  
    try
    {

#ifdef WIN32
#   if defined(_DEBUG)
        setName( pThr->m_name.c_str() );
#   endif

#   ifndef __BCPLUSPLUS__
        CSE::MapSEtoCE(); /* Must be called before any exceptions are raised */
#   endif           
        if( pThr->setTranslator_ )
            attachExceptionTranslator();
#endif /* WIN32 */

        launcher2( pThr );
    }
    catch( const Exception& ex )
    {
        cerr << "\n--- APPLICATION EXCEPTION in Thread::run()!!! ---, name="  << pThr->m_name << endl
            << ex.what() << ", id=" << self() << endl;
        assert(! "APPLICATION EXCEPTION in Thread::run()");
    }
    catch(const std::exception& ex)
    {
        cerr << "\n--- std EXCEPTION in Thread::run()!!! ---\n" << ex.what() << ", id=" << self() << endl;
        assert(! "std EXCEPTION in Thread::run()");
    }

#ifdef WIN32
#ifndef _DEBUG
#ifndef __BCPLUSPLUS__
    catch (CSE se) {
        switch (se) {    /* Calls the operator DWORD() member function */
        case EXCEPTION_ACCESS_VIOLATION:
            /* This code handles an access-violation exception */
            char msg[1024];
            sprintf(msg, "EXCEPTION_ACCESS_VIOLATION, id=%d, name=%s", self(), pThr->m_name.c_str());
            ::OutputDebugString(msg);
            clog << msg << endl;
            DebugBreak();
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            /* This code handles a division-by-zero exception */
            clog << "EXCEPTION_INT_DIVIDE_BY_ZERO, id=" << self() << endl;
            break;
        default:
            /* We don't handle any other exceptions */
            throw;   /* Maybe another catch is looking for this */
            break;   /* Never executes */
        }
        abort();
    }
#endif /* __BCPLUSPLUS__ */
#endif /*  _DEBUG */
#endif /* WIN32 */

#ifndef _DEBUG
    catch(...)
    {
        //cerr << "Unhandled exception in Thread::launcher, id=" << self() << endl;
        exit( EXIT_FAILURE );
    }
#endif /* _DEBUG */


#ifndef WIN32
    pthread_exit(0);
#endif /* WIN32 */
}

void Thread::start()
{
    Guard<Mutex> lock(lock_);

    /*
       Ensure that thread was joined before starting.
       It is useful when user wants to restart thread and forgets to call join,
       or join can't be performed because of self context.
    */
    join();

#ifndef WIN32
    pthread_attr_t attr;
    i32 rv = pthread_attr_init(&attr);
    if( 0 != rv )
    {
        throw system_exception("pthread_attr_init", rv);
    }

    rv = pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
    if( 0 != rv )
    {
        throw system_exception("pthread_attr_setscope", rv);
    }

    i32 threaderr = pthread_create( &m_id, &attr, startHook, this );
    if( 0 != threaderr )
    {
        throw system_exception("pthread_create", threaderr);
    }

    rv = pthread_attr_destroy( &attr );
    if( 0 != rv )
    {
        throw system_exception("pthread_attr_destroy", rv);
    }
#else
    m_handle = reinterpret_cast<HANDLE>( _beginthreadex(NULL, 0, &startHook, this, 0, &m_id) ); 
    if( 0 == m_handle )
    {
        throw system_exception("_beginthreadex", GetLastError());
    }
#endif /* WIN32 */
}

bool Thread::equals( ThreadId thrId )
{
#ifndef WIN32
    return pthread_equal(getId(), thrId) != 0;
#else
    return getId() == thrId;
#endif
}

void Thread::join() 
{
    if( !equals( self() ) )
    {
        Guard<Mutex> lock(lock_);

#ifndef WIN32
        if( m_id != 0 )
        {
            int rv = pthread_join( m_id, NULL );

            if( rv != 0 )
                throw system_exception("pthread_join() failed", rv);

            m_id = 0;
        }
#else 
        if( m_handle != 0 )
        {
            if( WaitForSingleObject(m_handle, INFINITE) == WAIT_FAILED )
                throw system_exception("WaitForSingleObject failed", GetLastError());

            cleanup();
        }
#endif /* WIN32 */
    }
}

void Thread::yield()
{
#ifndef WIN32
    if( 0 != sched_yield() )
    {
        throw system_exception("sched_yield", errno);
    }
#else 
    Sleep(0);
#endif /* WIN32 */
}

ThreadId Thread::self()
{
#ifndef WIN32
    return pthread_self();
#else 
    return GetCurrentThreadId();
#endif /* WIN32 */
}

void Thread::cancel()
{
#ifndef WIN32
    if( m_id != 0 )
    {
        i32 res = pthread_cancel(m_id);
        if( 0 != res )
        {
            throw system_exception("pthread_cancel", res);
        }
    }
#else
    if( m_handle != 0 )
    {
        if( !TerminateThread(m_handle, 0) )
            throw system_exception("Thread cancellation failed", ERRNO);

        join();
    }
#endif /* WIN32 */
}

bool Thread::sleep( u32 aMilis )
{
#ifndef WIN32
    struct timeval tValue;
    tValue.tv_sec = aMilis / 1000;
    tValue.tv_usec = (aMilis % 1000) * 1000; // microseconds

    if(0 != select(0,0,0,0, &tValue)){
        if(EINTR == errno){
            return false;
        }else{
            throw system_exception("nanosleep", errno);
        }
    }
    return true;
#else
    Sleep( aMilis );
    return true;
#endif /* WIN32 */
}

void Thread::setCancelMode( CancellationType aMode ) 
{
#ifndef WIN32
    if(DISABLED == aMode)
    {
        i32 ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if( 0 != ret )
        {
            throw system_exception("pthread_setcancelstate", ret);
        }
    }
    else
    {
        i32 ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        if( 0 != ret )
        {
            throw system_exception("pthread_setcancelstate", ret);
        }

        if( DEFERRED == aMode )
        {
            i32 ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
            if( 0 != ret )
            {
                throw system_exception("pthread_setcanceltype", ret);
            }
        }
        else
        { /* ASYNCHRONOUS */
            i32 ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
            if( 0 != ret )
            {
                throw system_exception("pthread_setcanceltype", ret);
            }
        }
    }
#else 
    aMode;
    assert( false );
#endif /* WIN32 */
}

void Thread::interrupt()
{
#ifndef WIN32
    i32 ret = pthread_kill( m_id, s_INTERRUPT_SIGNAL );
    if( 0 != ret )
    {
        throw system_exception("pthread_kill", ret);
    }
#else 
    if( m_handle != 0 )
    {
        if( !TerminateThread(m_handle, 0) )
            throw system_exception("Thread interruption failed", ERRNO);
        join();
    }
#endif /* WIN32 */

}

#if defined( WIN32 ) && defined(_MSC_VER)

void Thread::setName( s8 const* szThreadName )
{
    s8 buf[512];
    sprintf_s( buf, sizeof(buf)-1, "Worker: %s", szThreadName );
    buf[511] = '\0';
     
    const i32 MS_VC_EXCEPTION = 0x406d1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;        /* must be 0x1000 */
        LPCSTR szName;       /* pointer to name (in same addr space) */
        DWORD dwThreadID;    /* thread ID (-1 caller thread) */
        DWORD dwFlags;       /* reserved for future use, most be zero */
    } THREADNAME_INFO;

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = &buf[0];
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD),
            (ULONG_PTR *)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}
#else

void Thread::setName( s8 const* /*szThreadName*/ )
{
}
#endif /* defined( WIN32 ) && defined(_MSC_VER) */

