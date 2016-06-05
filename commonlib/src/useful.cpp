#include "useful.h"
#include <cassert>
#include <stdio.h>

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#include "mutex.h"
#endif

using namespace std;

/*
string i2str( i32 val )
{
    bool negative = (val < 0);
    if( negative )
        val *= -1;

    s8 pos = 63;
    s8 buf[64];
    buf[pos--] = 0;
    do {
        buf[pos--] = 48 + val%10;
        val = val/10;
    }
    while( 0 != val );

    if( negative )
        buf[pos--] = '-';
    return &buf[pos + 1];
}

string u2str( u32 val )
{
    s8 pos = 63;
    s8 buf[64];
    buf[pos--] = 0;
    do {
        buf[pos--] = 48 + val%10;
        val = val/10;
    }
    while( 0 != val );
    return &buf[pos + 1];
}
*/


#ifndef WIN32
static Mutex stdinLock_;
#endif

i32 getch( void )
{
    i32 ch = 0;

#ifndef WIN32
    MGuard g(stdinLock_);

    struct termios term;
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    ch = getchar();

    term.c_lflag |= (ICANON|ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &term );
#else
    u8 i = 0;
    s8* chbuf = (s8*)&ch;
    do{ chbuf[i++] = (s8)_getch(); } while( _kbhit() );
#endif
//    printf("fread: %d %d %d %d\n", chbuf[0], chbuf[1], chbuf[2], chbuf[3]);
//    printf("getch returns %d\n", ch);
    return ch;
}

string u2hex( u32 val, s16 width )
{
   if( width <= 0 )
        width = 0;
    width--;

    s16 pos = 63;

    s8 buf[64];
    buf[pos--] = 0;
    do {
        s16 diff = val%16;
        diff += (diff > 9) ? 55 : 48;
        buf[pos--] = (s8)diff;
        val = val/16;
    }
    while( width-- > 0 || 0 != val );

    return &buf[pos + 1];
}

u16 split( const std::string& in_str, 
           s8 delimiter, 
           StringsT* pOut )
{
    string::size_type prevPsn = 0, psn = 0;
    while( (psn = in_str.find(delimiter, prevPsn)) != string::npos )
    {
        if( psn != prevPsn )
        {                
            pOut->push_back( in_str.substr(prevPsn, psn - prevPsn) );
        }
        prevPsn = psn + 1;
    }

    if( prevPsn != in_str.size() )
    {
        pOut->push_back(in_str.substr(prevPsn));
    }
    return static_cast<u16>(pOut->size());
}

u16 split( const std::string& in_str, 
           const std::string& delimiter, 
           StringsT* pOut )
{
    string::size_type prevPsn = 0, psn = 0;
    while( (psn = in_str.find( delimiter, prevPsn )) != string::npos )
    {
        if( psn != prevPsn )
        {                
            pOut->push_back(in_str.substr(prevPsn, psn - prevPsn));
        }
        prevPsn = psn + delimiter.size();
    }
    if( prevPsn != in_str.size() )
    {
        pOut->push_back(in_str.substr(prevPsn));
    }
    return static_cast<u16>(pOut->size());
}

void trimWhiteSpace(std::string* pStr)
{
    string::size_type pos = 0;    
    if( (pos = pStr->find_last_not_of(" \t\n")) != string::npos )
        pStr->resize(pos+1);
    else 
        pStr->resize(0);

    pos = pStr->find_first_not_of(" \t\n");

    if( pos != 0 && pos != string::npos )
        pStr->erase(0, pos);
}


namespace 
{
    template< typename INT_T>
    inline s8* uitoa_impl(mem_block buffer, INT_T val)
    {
        typename IntTraints<INT_T>::Unsigned value = static_cast<typename IntTraints<INT_T>::Unsigned>( val );
        assert( nullptr != buffer.ptr_ );
        assert( nullptr != buffer.end_ );
        s8* end = buffer.last();

        do
        {
            unsigned int remainder = static_cast<unsigned int>( value % 10 );
            value /= 10;
            assert( end >= buffer.ptr_ );
            *end-- = static_cast<s8>( remainder + '0' );
        }
        while( 0 != value );
        ++end;
        return end;
    }

    template< typename INT_T>
    inline s8* itoa_impl(mem_block buffer, INT_T val)
    {
        typename IntTraints<INT_T>::Signed value = static_cast<typename IntTraints<INT_T>::Signed>( val );
        assert( nullptr != buffer.ptr_ );
        assert( nullptr != buffer.end_ );

        if( value < 0 )    
        {
            s8* result = uitoa_impl( buffer, static_cast<typename IntTraints<INT_T>::Unsigned>( -value ) );
            --result;
            assert( result >= buffer.ptr_ );
            *result = '-';
            return result;
        }
        return uitoa_impl( buffer, static_cast<typename IntTraints<INT_T>::Unsigned>( value ) );
    }

    template< typename UInt, typename CharT >
    inline UInt atou_impl(CharT const* str, CharT const** end)
    {
        assert( nullptr != str );
        assert( nullptr != end );
        UInt result = 0;
        while( str != *end )
        {
            UInt c = *str - '0';
            if( c > 9 )
            {
                *end = str;
                return result;
            }
            result = result * 10 + c;
            ++str;
        }
        return result;
}

    template< typename Int, typename CharT >
    inline Int atoi_impl(CharT const* str, CharT const** end)
    {
        assert( nullptr != str );
        assert( nullptr != end );
        if(*end == str ) return 0;
        bool negative = '-' == str[0];
        if( negative ) 
        {
            ++str;
        }

        Int result = 0;
        while( str != *end )
        {
            typename IntTraints<Int>::Unsigned c = *str - '0';
            if( c > 9 )
            {
                *end = str;
                return negative ? -result : negative;
            }
            result = result * 10 + c;
            ++str;
        }
        return negative ? -result : result;
    }

    /* Size of the maximum int32 value in string representation */
    static u32 const MAX_INT32_STR_SIZE = sizeof("-2147483647") - 1;

    /* Size of the maximum uint32 value in string representation */
    static u32 const MAX_UINT32_STR_SIZE = sizeof("4294967296") - 1;

    /* Size of the maximum int64 value in string representation */
    static u32 const MAX_INT64_STR_SIZE = sizeof("-9223372036854775808") - 1;

    /* Size of the maximum uint64 value in string representation */
    static u32 const MAX_UINT64_STR_SIZE = sizeof("18446744073709551616") - 1;

    /* Size of the maximum uint64 value in string representation */
    static u32 const MAX_SIZE_T_STR_SIZE = MAX_UINT64_STR_SIZE;
}

s8* i64toa(s8* pBuf, u8 buf_size, i64 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = pBuf + buf_size;
    return itoa_impl( buf, value );
}

s8* i64toa(s8* pBuf, s8* end, i64 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = end;
    return itoa_impl( buf, value );
}

s8* u64toa(s8* pBuf, u8 buf_size, u64 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = pBuf + buf_size;
    return uitoa_impl( buf, value );
}

s8* u64toa(s8* pBuf, s8* end, u64 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = end;
    return uitoa_impl( buf, value );
}

s8* i32toa(s8* pBuf, u8 buf_size, i32 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = pBuf + buf_size;
    return itoa_impl( buf, value );
}

s8* u32toa(s8* pBuf, u8 buf_size, u32 value)
{
    mem_block buf;
    buf.ptr_ = pBuf;
    buf.end_ = pBuf + buf_size;
    return uitoa_impl( buf, value );
}

s8* u32toa(s8* pBuf, s8* end, u32 value)
{
    return u32toa( pBuf, end - pBuf, value );
}

std::string tostring( u32 value )
{
    s8 str_buf[ MAX_UINT32_STR_SIZE ];
    return std::string( u32toa( str_buf, MAX_UINT32_STR_SIZE, value ), &str_buf[ MAX_UINT32_STR_SIZE ] );
}

std::string tostring( i32 value )
{
    s8 str_buf[ MAX_INT32_STR_SIZE ];
    return std::string( i32toa( str_buf, MAX_INT32_STR_SIZE, value ), &str_buf[ MAX_INT32_STR_SIZE ] );
}

std::string tostring( u64 value )
{
    s8 str_buf[ MAX_UINT64_STR_SIZE ];
    return std::string( u64toa( str_buf, MAX_UINT64_STR_SIZE, value ), &str_buf[ MAX_UINT64_STR_SIZE ] );
}

std::string tostring( i64 value )
{
    s8 str_buf[ MAX_INT64_STR_SIZE ];
    return std::string( i64toa( str_buf, MAX_INT64_STR_SIZE, value ), &str_buf[ MAX_INT64_STR_SIZE ] );
}

#ifdef WIN32
#include <windows.h>
/*
std::string strerror( int errnum )
{
    s8 buf[1024];
    strerror_s(buf, 1023, errnum);
    return std::string(buf);
}
*/
s32 gettimeofday( struct timeval* tp, void* ptimezone )
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    const unsigned long long EPOCH = 116444736000000000ULL;

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    unsigned long long time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((unsigned long long)file_time.dwLowDateTime )      ;
    time += ((unsigned long long)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#else
#define strerror strerror
#endif

#ifdef WIN32
void split_path( const s8* path, s8* drive, s8* dir, s8* fname, s8* ext )
{
    _splitpath_s( path, 
                  drive, (NULL != drive)?(INT_MAX):(0), 
                  dir, (NULL != dir)?(INT_MAX):(0), 
                  fname, (NULL != fname)?(INT_MAX):(0), 
                  ext, (NULL != ext)?(INT_MAX):(0));
}
#endif

/*************************************************************************/
// 100 nanoseconds between 1960.01.01-00:00:00 and 1970.01.01-00:00:00
u64 const WIN_TIME_CORRECTOR = 116444736000000000ull;

u64 current_time() 
{
#ifndef WIN32
	struct timeval tValue;
	if(0 != gettimeofday(&tValue, NULL) )
    {
		throw system_exception("gettimeofday", errno);
	} 

	u64 sec, uSec, rv;
	sec   = tValue.tv_sec;
	uSec  = tValue.tv_usec;
	sec  *= 1000UL;
	uSec /= 1000UL;
	rv    = sec +  uSec;
	return rv;
#else 
	FILETIME time;
	GetSystemTimeAsFileTime(&time);
	ULARGE_INTEGER v;
	v.LowPart = time.dwLowDateTime;
	v.HighPart = time.dwHighDateTime;
	return (v.QuadPart - WIN_TIME_CORRECTOR) / 10000;
#endif
}
