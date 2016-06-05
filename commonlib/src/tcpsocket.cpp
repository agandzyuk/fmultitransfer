#ifndef WIN32
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

#include "tcpsocket.h"
#include "useful.h"

struct TCPSocket::Impl
{
    Impl() : shutdownWasCalled_(false)
    {};
    bool shutdownWasCalled_;    
};

TCPSocket::TCPSocket(SD fd) 
    : Socket(fd), 
    pImpl_(NULL)
{
    pImpl_ = new Impl();
}

TCPSocket::TCPSocket()
    : Socket(AF_INET, SOCK_STREAM, 0) 
{
    pImpl_ = new Impl();
}

TCPSocket::~TCPSocket()
{
    if( !pImpl_->shutdownWasCalled_ )
    {
        try {
            shutdown(BOTH);
        }
        catch( const Exception& )
        {}
    }    
    delete pImpl_;
}

void TCPSocket::close()
{
    shutdown( BOTH );
    Socket::close();
}

void TCPSocket::shutdown( How how )
{
    if( BOTH == how )
    {
        pImpl_->shutdownWasCalled_ = true;
    }

    if( 0 != ::shutdown( m_fd, how ) )
    {
        int errorCode = SOCKET_ERRNO;
        if( ERR_ENOTCONN != errorCode )
        {    
            throw system_exception("::shutdown", errorCode);	
        }
    }
}

void TCPSocket::setTcpNoDelay( bool on )
{
    u32 aOn = on;
    if( 0 != ::setsockopt( m_fd,     
                           IPPROTO_TCP, 
                           TCP_NODELAY, 
                           (const s8*)&aOn, 
                          sizeof(aOn)) )
    {
        throw system_exception("TCPSocket::setTcpNoDelay(" + tostring(on) + ")", SOCKET_ERRNO);
    }
}

bool TCPSocket::getTcpNoDelay()
{
    u32 on = 0;
    socklen_t size = sizeof(on);
    if( 0 != ::getsockopt( m_fd, 
                           IPPROTO_TCP, 
                           TCP_NODELAY, 
                           (s8*)&on, 
                           &size) )
    {
        throw system_exception("TCPSocket::getTcpNoDelay() failed", SOCKET_ERRNO);
    }
    return (on == 1);
}
