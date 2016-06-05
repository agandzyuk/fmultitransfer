#ifndef WIN32
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#endif 

#include "tcpclient.h"

using namespace std;

TCPSockClient::TCPSockClient() 
    : is_open_(false) 
{}


TCPSockClient::TCPSockClient(SD fd) 
    : TCPSocket(fd), 
    is_open_(true) 
{}

bool TCPSockClient::connect(const IPAddress& address, u16 port) 
{
    address_ = address;

    if( is_open_ ) 
        return true; 
    open(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr   = address_.get_address();
    sockAddr.sin_port   = htons(port);

    is_open_ = ::connect(m_fd, reinterpret_cast<struct sockaddr*>(&sockAddr), sizeof(sockAddr)) == 0;
    if( !is_open_ ) 
    {
        const s32 errCode = SOCKET_ERRNO;
        if( ERR_WOULDBLOCK != errCode )
        {
            throw system_exception("connect() to (" + address_.getHostAddress() +
                                   ":" + tostring(port) + ") failed", errCode);
        }
        return false;
    }

    if(target_.empty())
        target_  = address.getHostName();
    return true;
}

TCPSockClient::TCPSockClient(const IPAddress& address, u16 port)
    : is_open_(false)
{
    connect(address, port);
}

void TCPSockClient::close() 
{
    Socket::close();
    is_open_ = false;
    target_.clear();
}

IPAddress& TCPSockClient::getIPAddress() const
{
    if( address_.get_address().s_addr == 0)
    {
        struct sockaddr_in addr;
        socklen_t nameLen = sizeof(addr);
        if( 0 != getpeername(m_fd, (sockaddr*)&addr, &nameLen ))
            throw system_exception("getpeername", SOCKET_ERRNO);
        address_ = addr.sin_addr;
    }
    return address_;
}

u16 TCPSockClient::get_port() const 
{
    struct sockaddr_in addr;
    socklen_t nameLen = sizeof (addr);
    if( 0 != getpeername(m_fd, (sockaddr*)&addr, &nameLen) )
        throw system_exception("getpeername", SOCKET_ERRNO);
    return ntohs( addr.sin_port );
}

s32 TCPSockClient::send(const void* msg, s32 len) 
{
#ifndef WIN32
    s32 ret = ::send(m_fd, msg, len, 0);
#else 
    s32 ret = ::send(m_fd, (const s8*)msg, len, 0); 
#endif 
    if( ret == SOCKET_ERROR )
    {    
        s32 errorCode = SOCKET_ERRNO;
        if( ERR_WOULDBLOCK == errorCode )
            return -1;    
        else
            throw system_exception("::send", errorCode);
    }
    return ret;
}

s32 TCPSockClient::recv( void* buf, s32 len )
{
#ifndef WIN32
    s32 ret = ::recv(m_fd, buf, len, 0);
#else 
    s32 ret = ::recv(m_fd, (s8*)buf, len, 0);
#endif 
    if( SOCKET_ERROR == ret )
    {
        s32 errorCode = SOCKET_ERRNO;
        if( ERR_WOULDBLOCK == errorCode )
        {
            return -1;
        }
#ifdef WIN32
        else if( errorCode == ERR_ETIMEDOUT )
#else
        else if( errorCode == ERR_EINTR )
#endif
        {
            return 0;
        }
        else
        {       
            throw system_exception("::recv", errorCode);
        }
    }
    return ret;
}

s32 TCPSockClient::availableToRead() 
{
#ifndef WIN32
    s32 availBytes = 0;    
    if( -1 == ::ioctl(m_fd, FIONREAD, &availBytes) ) 
        throw system_exception("ioctl", SOCKET_ERRNO);
#else
    unsigned long availBytes = 0;
    if( 0 != ioctlsocket(m_fd, FIONREAD, &availBytes ) )
        throw system_exception("ioctlsocket, FIONREAD", SOCKET_ERRNO);
#endif 
    return (s32)availBytes;
}

bool TCPSockClient::until_connected( struct timeval* timeout )
{
    bool result = untilReadyToWrite( timeout );
    if ( result )
        is_open_ = true;
    return result;
}
