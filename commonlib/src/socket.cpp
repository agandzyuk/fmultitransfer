#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <sys/capability.h>
// #include <sys/prctl.h>
#include <unistd.h>
#elif _MSC_VER
#pragma comment( lib, "ws2_32.lib" )
#endif 

#include "socket.h"
#include "useful.h"

struct Socket::Impl
{
    Impl() : is_closed_(false)
    {}

    bool is_closed_;
};

Socket::Socket() 
    : m_fd(-1), 
    pImpl_(NULL)
{
    pImpl_ = new Impl();
    pImpl_->is_closed_ = true;
}

Socket::Socket(SD aFd) 
    : m_fd(aFd), 
    pImpl_(NULL)
{
    pImpl_ = new Impl();
}

Socket::Socket(u32 domain, u32 type, u32 protocol)
    : pImpl_(NULL)
{
    m_fd = socket(domain, type, protocol);
    if(INVALID_SOCKET  == m_fd)
    {
        throw system_exception("::socket() failed", SOCKET_ERRNO);
    }
    pImpl_ = new Impl();
}

Socket::~Socket()
{
    if( !pImpl_->is_closed_ )
    {
        try {
            close();
        }
        catch( const Exception& )
        {}
    }
    delete pImpl_;
}

void Socket::open(u32 domain, u32 type, u32 protocol) 
{
    if( pImpl_->is_closed_ ) 
    {
        m_fd = socket(domain, type, protocol);
        if( INVALID_SOCKET  == m_fd )
        {
            throw system_exception("::socket() failed", SOCKET_ERRNO);
        }
        pImpl_->is_closed_ = false;
    }
}

void Socket::close()
{
    if( !pImpl_->is_closed_ )
    {
        pImpl_->is_closed_ = true;
#ifndef WIN32
        if( 0 != ::close(m_fd) )
        {
#else 
        if( 0 != ::closesocket(m_fd) )
        {
#endif
            throw system_exception("::close", SOCKET_ERRNO);
        }
        m_fd = INVALID_SOCKET;
    }
}

IPAddress Socket::getLocalAddress() const 
{
    struct sockaddr_in addr;
    socklen_t nameLen = sizeof(addr);
    if( 0 != getsockname(m_fd, (sockaddr* )&addr, &nameLen) )
    {
        throw system_exception("getsockname", SOCKET_ERRNO);
    }
    return IPAddress( addr.sin_addr );
}

u16 Socket::getLocalPort() const 
{
    struct sockaddr_in addr;
    socklen_t nameLen = sizeof (addr);
    if( 0 != getsockname(m_fd, (sockaddr* )&addr, &nameLen) )
    {
        throw system_exception("getsockname", SOCKET_ERRNO);
    }
    return ntohs( addr.sin_port );
}

bool Socket::untilReadyToWrite(struct timeval* timeout) 
{
    fd_set set;
    FD_ZERO( &set );
    FD_SET( m_fd, &set );
    int ret = ::select( static_cast<int>(m_fd+1), 
                        NULL, 
                        &set, 
                        NULL, 
                        timeout ); 
    if( 0 < ret )
        return true;
    else if( SOCKET_ERROR == ret )
        throw system_exception("::select", SOCKET_ERRNO); 
    return false;
}

bool Socket::untilReadyToRead(struct timeval* timeout) 
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(m_fd, &set);
    int ret = ::select( static_cast<int>(m_fd+1), 
                        &set, 
                        NULL, 
                        NULL, 
                        timeout ); 
    if( 0 < ret )
        return true;
    else if( SOCKET_ERROR == ret )
        throw system_exception("::select", SOCKET_ERRNO); 
    return false;
}

void Socket::set_option( u32 level, u32 name, const s8* value,  socklen_t length )
{
    assert(INVALID_SOCKET  != m_fd);
    if(0 != setsockopt( m_fd, 
                        level, 
                        name, 
                        value, 
                        length ) )
    {
        throw system_exception("::setsockopt("  + tostring(level) + ", " + tostring(name)
                           + ",..) failed()", SOCKET_ERRNO);    
    }
}

void Socket::get_option(u32 level, u32 name, s8* value, socklen_t* length)
{
    assert(INVALID_SOCKET  != m_fd);
    if(0 != getsockopt( m_fd, 
                        level, 
                        name, 
                        value, 
                        length ) )
    {
        throw system_exception("::getsockopt(" + tostring(level) + ", " + tostring(name)
                           + ",..) failed", SOCKET_ERRNO);  
    }
}

int Socket::getLastError()
{
    int result;
    socklen_t len = sizeof( result );
    get_option( SOL_SOCKET, 
                SO_ERROR, 
                reinterpret_cast<char*>(&result), 
                &len );
    assert(len == sizeof(result));
    return result;
}

bool Socket::isValidSd(SD fd)
{
    s8 value = 0; 
    socklen_t length = 1;
    if(0 == getsockopt( fd, 
                        SOL_SOCKET, 
                        SO_DEBUG, 
                        &value, 
                        &length ))
    {
        return true;
    }

#ifdef WIN32
    const int BAD_FD_ERROR = WSAENOTSOCK;
#else
    const int BAD_FD_ERROR = EBADF;
#endif

    if(BAD_FD_ERROR != SOCKET_ERRNO)
    {
        return true;
    }
    return false;
}

void Socket::set_nonblocking(bool on)
{
#ifndef WIN32
    s32 flags;
    if( (flags = fcntl(m_fd, F_GETFL, 0)) < 0 )
    {
        throw system_exception("::fcntl, F_GETFL", SOCKET_ERRNO);
    }
    if( on )
       flags |= O_NONBLOCK;
    else
       flags &= ~O_NONBLOCK;

    if( fcntl(m_fd, F_SETFL, flags) < 0 )
    {
        throw system_exception("::fcntl, F_SETFL", SOCKET_ERRNO);
    }
#else 
    unsigned long ul = (on) ? 1 : 0;
    if( 0 != ioctlsocket( m_fd, 
                          FIONBIO, 
                          &ul ) )
    {
        throw system_exception("::ioctlsocket", SOCKET_ERRNO);
    }
#endif 
}

void Socket::setReuseAddress(bool on)
{
#ifndef WIN32
    i32 st = on == true ? 1 : 0;
    if( setsockopt( m_fd,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    &st,
                    sizeof(st) ) < 0 )
    {
        throw system_exception("setsockopt, SO_REUSEADDR", SOCKET_ERRNO);
    }
#else
    BOOL st = on == true ? TRUE : FALSE;
    if( setsockopt( m_fd,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const s8*)&st,
                    sizeof(st) ) < 0 )
    {
        throw system_exception("setsockopt, SO_REUSEADDR", SOCKET_ERRNO);
    }
#endif
}

#ifndef WIN32
void Socket::setReusePort(bool on)
{
    i32 st = on == true ? 1 : 0;
    if( setsockopt( m_fd,
                    SOL_SOCKET,
                    SO_REUSEPORT,
                    &st,
                    sizeof(st) ) < 0 )
    {
        throw system_exception("setsockopt, SO_REUSEPORT", SOCKET_ERRNO);
    }
}
#endif

void Socket::setSendBufferSize(u32 size) 
{
    set_option( SOL_SOCKET, 
                SO_SNDBUF, 
                (const s8* )&size, 
                sizeof(size) );
}

u32 Socket::getSendBufferSize() 
{
    u32 bufSize   = 0;
    socklen_t len = sizeof(bufSize);
    get_option( SOL_SOCKET, 
                SO_SNDBUF, 
                (s8*)&bufSize, 
                &len );
    return bufSize;
}

void Socket::setReceiveBufferSize(u32 size)
{
    set_option( SOL_SOCKET, 
                SO_RCVBUF, 
                (const s8*)&size, 
                sizeof(size) );
}

u32 Socket::getReceiveBufferSize()
{
    u32 bufSize   = 0;
    socklen_t len = sizeof(bufSize);
    get_option( SOL_SOCKET, 
                SO_RCVBUF, 
                (s8*)&bufSize, 
                &len );
    return bufSize;
}

void Socket::setReceiveTimeout(u32 seconds, u32 uSeconds)
{
    timeval timeout;
    timeout.tv_sec  = seconds;
    timeout.tv_usec = uSeconds;
    set_option( SOL_SOCKET, 
                SO_RCVTIMEO, 
                (const s8*)&timeout, 
                sizeof timeout );    
}

void Socket::getReceiveTimeout(u32* pSeconds, u32* pUSeconds)
{
    timeval timeout;
    timeout.tv_sec = timeout.tv_usec = 0;
    socklen_t len  = sizeof(timeout);
    get_option( SOL_SOCKET,
                SO_RCVTIMEO, 
                (s8*)&timeout, 
                &len );
    *pSeconds  = timeout.tv_sec;
    *pUSeconds = timeout.tv_usec;
}

void Socket::setSendTimeout(u32 seconds, u32 uSeconds)
{
    timeval timeout;
    timeout.tv_sec  = seconds;
    timeout.tv_usec = uSeconds;
    set_option( SOL_SOCKET, 
                SO_SNDTIMEO, 
                (const s8*)&timeout, 
                sizeof(timeout) );    
}

void Socket::getSendTimeout(u32* pSeconds, u32* pUSeconds)
{
    timeval timeout;
    timeout.tv_sec = timeout.tv_usec = 0;
    socklen_t len  = sizeof(timeout);

    get_option( SOL_SOCKET, 
                SO_SNDTIMEO, 
                (s8*)&timeout, 
                &len );
    *pSeconds  = timeout.tv_sec;
    *pUSeconds = timeout.tv_usec;
}

int Socket::getLastNetworkError()
{
    return SOCKET_ERRNO;
}
