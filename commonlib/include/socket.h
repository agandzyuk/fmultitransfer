#ifndef __socket_h__
#define __socket_h__

#include "system_exception.h"
#include "ipaddress.h"

#ifndef WIN32
    typedef i32 SD; 
#else 
    typedef SOCKET SD;
    typedef i32 socklen_t;
#endif 

/*  This class represents client sockets (also called just "sockets"). 
    A socket is an endpoint for communication between two machines. 
*/
class Socket
{
public:

    enum How {
        RECEIVE = 0, 
        SEND    = 1, 
        BOTH    = 2,
    };

    /*  Returns the local address to which the socket is bound.
        Under WIN32 valid only for connected sockets.
    */
    IPAddress getLocalAddress() const;

    /*  Returns the local port number to which this socket is connected. */
    u16 getLocalPort() const;

    /* Closes this socket */
    virtual void close() ;

    /* Returns socket's descriptor. */
    SD get_fd() const 
    { return m_fd; }

    /*  Blocks until the socket is ready to read.
        @return true when socket to read, false when timeout ends
    */
    bool untilReadyToRead(struct timeval* timeout = NULL);

    /*  Blocks until the socket is ready to write.
	    @return true when socket to write, false when timeout ends
    */
    bool untilReadyToWrite(struct timeval* timeout = NULL);

    /*  Sets the socket options. */
    void set_option(u32 level, u32 name, const s8* pVal, socklen_t len);

    /*  Returns the socket options. */
    void get_option(u32 level, u32 name, s8* pVal, socklen_t* pLen);

    /*  Sets the socket in nonblocking mode.    */
    void set_nonblocking( bool on = true );

    /*  Sets the SO_REUSEADDR option to the specified value. */
    void setReuseAddress(bool on);

#ifndef WIN32
    /*  Sets the SO_REUSEPORT option to the specified value.
        @note Linux specific.
    */
    void setReusePort(bool on);
#endif

    /*  Sets the SO_SNDBUF option to the specified value. */
    void setSendBufferSize(u32 size);

    /*  Returns the value of the SO_SNDBUF option. */
    u32 getSendBufferSize();
    
    /*  Retrieves error status and clear. */
    i32 getLastError();

    /*  Retrieves last error status of network subsystem. */
    static i32 getLastNetworkError();
   
    /*  Sets the SO_RCVBUF option to the specified value.
        @note When setting the size of the TCP socket receive buffer, 
        the ordering of the function calls is important. This is because of
        TCP's window scale option, which is exchanged with the peer on the SYN
        segment when the connection is established. For a client, the SO_RCVBUF
        socket option must be set before calling ::connect().
        For a server, this means the socket option must be set for the listening
        socket befor calling ::listen(). Setting this option for the connected
        socket will have no effect whatsoever on the possible window scale 
        option because ::accept() does not return with the connected socket
        until TCP's three-way handshake is complete. That is why this option
        must be set for the listening socket. (The sizes of the socket buffers
        are always inhereted from the listening socket by the newly created
        connected socket.)
    */
    void setReceiveBufferSize(u32 size);

    /*  Returns the value of the SO_RCVBUF option.  */
    u32 getReceiveBufferSize();

    /*  Sets the SO_RCVTIMO option.     */
    void setReceiveTimeout(u32 seconds, u32 uSeconds);

    /*  Reads the SO_RCVTIMO option.    */
    void getReceiveTimeout(u32* pSeconds, u32* pUSeconds);

    /*  Sets the SO_SNDTIMO option.     */
    void setSendTimeout(u32 seconds, u32 uSeconds);

    /*  Reads the SO_SNDTIMO option.    */
    void getSendTimeout(u32* pSeconds, u32* pUSeconds);

    /*  Returns true if the given Socket Descriptor is valid, otherwise - false.    */
    static bool isValidSd(SD fd);

protected:

    /*  Creates an unconnected socket.  */
    Socket();

    /*  An unconnected socket may be created directly on the local machine. 
        Sockets can occupy both the internet domain (AF_INET) and UNIX 
        socket domain (AF_UNIX) under Unix. 
        The socket type (SOCK_STREAM, SOCK_DGRAM) and protocol may also
        be specified.
    */
    Socket(u32 domain, u32 type, u32 protocol);

    /*  A socket object may be created from a file descriptor when
        that descriptor was created either through a socket() or accept() call. 
        @param aFd the file descriptor of an already existing socket.
    */
    Socket(SD aFd);

    /*  Opens socket if it is closed.
        @throw Exception on errors.
    */
    void open(u32 domain, u32 type, u32 protocol);

    /*  Destructor. Closes this socket. */
    virtual ~Socket();

    SD m_fd;
    struct Impl; 
    Impl* pImpl_; 
};

#endif /* __socket_h__ */
