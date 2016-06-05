#ifndef __tcpsocket_h__
#define __tcpsocket_h__

#include "socket.h"

/*  Represents TCP socket.  */
class TCPSocket : public Socket
{
public:
	/*  Shutdowns the connection. 
        @throw system_exception
	 */
	void shutdown(How how);

	/*  Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).  */
	void setTcpNoDelay(bool enable = true);

	/*  Tests if TCP_NODELAY is enabled.
	 	@return true if TCP_NODELAY is enabled, otherwise false.
	*/
	bool getTcpNoDelay();

    /*  Reimplemented from Socket::close().
        Calls shutdown(BOTH).
    */
    virtual void close();

protected:

    TCPSocket();

    /*  Created TCPSocket from a given file descriptor 
        when that descriptor was created either through the ::socket() 
        or ::accept() system call. 
    */
    TCPSocket(SD fd);
    virtual ~TCPSocket();

private:
    struct Impl; 
    Impl* pImpl_; 
};

#endif /* __tcpsocket_h__ */
