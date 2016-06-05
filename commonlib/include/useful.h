#ifndef __useful_h__
#define __useful_h__

#include "common_types.h"
#include <string>
#include <vector>

#ifndef WIN32
    #define SC_ESC     27
    #define SC_ENTER   10
    #define SC_DEL     2117294875
    #define SC_BKSCP   127
    #define SC_UP      4283163
    #define SC_DOWN    4348699
    #define SC_LEFT    4414235
    #define SC_RIGHT   4479771
    #define SC_CTRLC   -1
    #define SC_CTRLZ   -2
#else
    #include <conio.h>

    #define SC_ESC     27
    #define SC_ENTER   13
    #define SC_DEL     21472
    #define SC_BKSCP   8
    #define SC_UP      18656
    #define SC_DOWN    20704
    #define SC_LEFT    19424
    #define SC_RIGHT   19936
    #define SC_CTRLC   3
    #define SC_CTRLZ   26
#endif

i32 getch( void );

/*
std::string i2str( i32 val );
std::string u2str( u32 val );
*/
std::string u2hex( u32 val, s16 width );

typedef std::vector<std::string> StringsT;
u16 split( const std::string& in_str, s8 delimiter, StringsT* out );
u16 split( const std::string& in_str, const std::string& delimiter, StringsT* out);
void trimWhiteSpace( std::string* pStr );

u8* i64toa( u8* pBuf, u8 buf_size, i64 value);
u8* i64toa( u8* pBuf, u8* end, i64 value);
u8* u64toa( u8* pBuf, u8 buf_size, u64 value);
u8* u64toa( u8* pBuf, u8* end, u64 value);
u8* i32toa( u8* pBuf, u8 buf_size, i32 value);
u8* u32toa( u8* pBuf, u8 buf_size, u32 value);
u8* u32toa( u8* pBuf, u8* end, u32 value);
std::string tostring( u32 value );
std::string tostring( i32 value );
std::string tostring( u64 value );
std::string tostring( i64 value );

/*  Returns the current UTC time in milliseconds.   */
u64 current_time();


#ifdef WIN32
s32 gettimeofday( struct timeval* tp, void* ptimezone );
/*
std::string strerror( int errnum );
*/
#endif

#ifdef WIN32

void split_path( const s8* path, s8* drive, s8* dir, s8* fname, s8* ext );
#define access _access

#else
#define _splitpath
#endif

/* Section for sockets error handling */
#ifndef WIN32
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#define SOCKET_ERRNO   (errno)
#define ERR_WOULDBLOCK EWOULDBLOCK
#define ERR_EINTR      EINTR
#define ERR_EINVAL     EINVAL
#define ERR_ENOTCONN   ENOTCONN
#define ERR_ETIMEDOUT  ETIMEDOUT
#define ERR_BADFD      EBADF
#else 
#define SOCKET_ERRNO   (WSAGetLastError())
#define ERR_WOULDBLOCK WSAEWOULDBLOCK
#define ERR_EINTR      WSAEINTR
#define ERR_EINVAL     WSAEINVAL
#define ERR_ENOTCONN   WSAENOTCONN
#define ERR_ETIMEDOUT  WSAETIMEDOUT
#define ERR_BADFD      WSAENOTSOCK
#endif 

#endif /* __useful_h__ */
