#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#endif 

#include "tcpserver.h"
#include "useful.h"

const int BACK_LOG = 1024;

WouldBlockException::WouldBlockException( const std::string &msg )
    : system_exception(msg, ERR_WOULDBLOCK)
{}

TCPSockServer::TCPSockServer(u16 port, 
                             const IPAddress* bind_addr, 
                             bool reuse_address) 
    : port_(port)
{
    struct sockaddr_in servAddr;
    memset( &servAddr, 0, sizeof(servAddr) );
    servAddr.sin_family = AF_INET;
    if( NULL == bind_addr )
        servAddr.sin_addr.s_addr = INADDR_ANY;
    else
        servAddr.sin_addr.s_addr = bind_addr->get_address().s_addr;
    servAddr.sin_port = htons( port_ );

    setReuseAddress( reuse_address );

    if( 0 != bind(m_fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) )
    {
        int errorCode = SOCKET_ERRNO;
        if(EINVAL == errorCode)
            throw WouldBlockException("::bind");
        else 
            throw system_exception("cannot bind to the port " + tostring(port_), SOCKET_ERRNO);
    }  
    if( 0 != listen(m_fd, BACK_LOG) )
        throw system_exception("listen", SOCKET_ERRNO);
}

TCPSockClient* TCPSockServer::accept()
{
    SD newFd = ::accept(m_fd, NULL, NULL);
#ifdef WIN32
    if( INVALID_SOCKET == newFd )
    {
#else
    if( -1 == newFd )
    {
#endif
        int errorCode = SOCKET_ERRNO;
        if( ERR_WOULDBLOCK == errorCode )
            throw WouldBlockException("::accept");
        else
            throw system_exception(
                "Cannot accept connection on port " + tostring(port_) + 
                ". Please check ListenPort parameter in engine properties file", 
                SOCKET_ERRNO
            );
    }
    return new TCPSockClient(newFd);
}
