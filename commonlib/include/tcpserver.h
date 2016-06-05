#ifndef __tcp_server_h__
#define __tcp_server_h__

#include "tcpclient.h"

/*******************************************************/
/*  This class implements server TCP socket. A server socket waits for 
    requests to come in over the network
*/
class TCPSockServer : public TCPSocket
{
public:  
    /*  Creates a server socket with the specified port and local IP address to bind to.
        If bind_addr is NULL, it will default accepting connections on any/all local addresses.
        @param reuse_address Allow or deny the socket to be bound to an address that is already in use.
        @throw system_exception
    */
    TCPSockServer(u16 port, 
                  const IPAddress* bind_addr = NULL, 
                  bool reuse_address = true);

    /*  Listens for a connection to be made to this socket and accepts it. 
        The method blocks until a connection is made. 
        A new TCPSockClient is created and returned. The user is responsible
        for it's deleting.
        @throw system_exception
    */
    TCPSockClient* accept();

private:
    u16 port_;
};

#endif /* __tcp_server_h__ */
