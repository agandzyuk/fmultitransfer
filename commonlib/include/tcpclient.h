#ifndef __tcp_client_h__
#define __tcp_client_h__

#include "tcpsocket.h"
#include "refcounted.h"

/*******************************************************/
class TCPSockServer;

/*******************************************************/
/*  This class implements client TCP socket.    */
class TCPSockClient : public TCPSocket, public RefCounted
{
    friend class TCPSockServer;

public:
    TCPSockClient( void );

    /*  Creates a stream socket and connects it to the specified port number 
        at the specified IP address.
        @throw system_exception
    */
    TCPSockClient(const IPAddress& address, u16 port);

    /*  Connects socket to the specified port number at the specified IP address.
        @throw Exception on errors.
        @note If socket is nonblocking the connection attempt cannot be completed immediately.
        Use until_connected() to determine the completion of the connection request.
    */
    bool connect(const IPAddress& address, u16 port);

    /*  Closes socket.
        @throw Exception on errors.
    */
    virtual void close();

    /*  Returns the remote port number to which this socket is connected.
        @throw system_exception
    */
    u16 get_port() const;

    /*  Transmit a message to another transport end-point. 
        If the socket does not have enough buffer space available to hold 
        the  message  being sent, this method blocks.
        @return the number of bytes that were sent, -1 if an error of EWOULDBLOCK was returned.
        @throw system_exception
    */
    s32 send(const void* msg, s32 len);

    /*  Receives a message from the socket.
        @return the number of bytes received, -1 if an error of EWOULDBLOCK was returned.
    */
    s32 recv(void* buf, s32 len);

    /*  Returns the number of bytes available to read. 
        You should avoid doing this because it is highly inefficient, 
        and it subjects an application to an incorrect data count. 
        @throw system_exception
    */
    s32 availableToRead();

    /*  Blocks until the socket is connected.
        @return true when socket is connected, false when timeout ends
    */
    bool until_connected(struct timeval* timeout = NULL);

    /*  Returns the remote IP address to which this socket is connected.
        @throw system_exception
    */
    IPAddress& getIPAddress() const;

    /*  Checks if socket opened.
        @return true if socket opened, false otherwise.
    */
    inline bool is_open() const
    { return is_open_; }

    /*  Returns the name of target IP address to which this socket is connected.    */
    inline const std::string& getTarget() const
    { return target_; }

    /* RefCounted implementation    */
    virtual s32 add_ref( void ) const 
    { return RefCounted::add_ref(); }
    
    virtual s32 release( void ) const 
    { return RefCounted::release(); }

protected:
    /*  Created TCPSockClient from a given file descriptor 
        when that descriptor was created either through a ::socket() 
        or ::accept() system call. 
    */
    TCPSockClient(SD fd);

private:
    /* Flag used to determine if socket is opened. */
    bool is_open_;
    mutable IPAddress address_;
    std::string target_;
};


#endif /* __tcp_client_h__ */
