#ifndef __system_defines_h__
#define __system_defines_h__

#include <cstdlib>

/* Sets prefered size for FD_SETSIZE */
#define REQUIRED_FD_SETSIZE 1024

#ifdef WIN32
#   ifdef FD_SETSIZE
#      error The default value of FD_SETSIZE cannot be used, please include this file before all other include files.
#   endif /* #ifdef FD_SETSIZE */
#   define FD_SETSIZE  REQUIRED_FD_SETSIZE

#   pragma warning(disable: 4786)

#   ifndef STRICT
#       define STRICT
#   endif

#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX 

#   ifndef _WIN32_WINNT
#       define _WIN32_WINNT 0x0500 /* Windows 2000 (NT 5.0 and later)*/
#   endif /* #ifndef _WIN32_WINNT */

#   include <winsock2.h>
#   include <windows.h>
#   include <WindowsX.h>

#   ifdef Yield /* Yield conflicts with FIX Field Name (Decorator problem) */
#       undef Yield
#   endif
#endif /* #ifdef WIN32 */

#include <cassert>

#ifndef WIN32

#include <termios.h>
#include <unistd.h>

/*  macro enables console echo in case assertions errors */
#ifndef __NO_ECHO_ASSERT
#define __NO_ECHO_ASSERT( expr )\
        if( expr )\
            __ASSERT_VOID_CAST (0);\
        else {\
            struct termios term;\
            tcgetattr( STDIN_FILENO, &term );\
            term.c_lflag |= ( ICANON | ECHO );\
            tcsetattr( STDIN_FILENO, TCSANOW, &term );\
        }
#endif


#ifndef __NO_ECHO_ASSERT_DBG
#define __NO_ECHO_ASSERT_DBG( expr )\
        if( expr )\
            __ASSERT_VOID_CAST (0);\
        else {\
            struct termios term;\
            tcgetattr( STDIN_FILENO, &term );\
            term.c_lflag |= ( ICANON | ECHO );\
            tcsetattr( STDIN_FILENO, TCSANOW, &term );\
            __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION);\
        }
#endif

#ifdef assert
#undef assert
#endif

#ifdef NDEBUG
    #define assert( expr ) __NO_ECHO_ASSERT( #expr )
#else
    #define assert( expr ) __NO_ECHO_ASSERT_DBG( #expr )
#endif /* NDEBUG */
#endif /* WIN32 */


#include <errno.h>
#ifndef byte
/* Declares alias for unsigned char */
typedef unsigned char byte;
#endif

#ifndef WIN32
#   ifdef __linux
#       include <sys/types.h>
#       include <string.h>
#       include <limits.h>
        typedef u_int64_t Long;
#   else /* __linux */
        /* Declares alias for unsigned long long */
        typedef unsigned long long Long;
#   endif /* __linux */
#else /* WIN32 */
    typedef ULONGLONG Long;
#endif


#if _MSC_VER >= 1500
#   define B2B_OVERRIDE override
#else
#   define B2B_OVERRIDE
#endif /* Compiler */

#endif /* __system_defines_h__ */
