#include "ipaddress.h"
#include "system_defines.h"

#ifndef WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

using namespace std;

const u32 ERROR_MSG_SIZE = 1024;

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

const u32 MAX_HOST_BUFFER_SIZE = MAXHOSTNAMELEN + 10 * sizeof(in_addr);

IPAddress::IPAddress()
{
    memset( &m_addr, '\0', sizeof(m_addr) );
}

string IPAddress::getHostAddress() const
{
    return string( inet_ntoa(m_addr) );
}

string IPAddress::getHostName() const
{
#ifndef WIN32
    s8 host[NI_MAXHOST];
    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));
    address.sin_addr = m_addr;
    address.sin_family = AF_INET;

    int rv = getnameinfo( (const sockaddr*) &address, 
                           sizeof(address), 
                           host, 
                           NI_MAXHOST, 
                           NULL, 0, 0 );

    if( 0 != rv )
    {
        s8 buf[ERROR_MSG_SIZE];
        sprintf(buf, "getnameinfo returned %d, reason: %s", rv, gai_strerror(rv));
        throw system_exception( buf, rv );
    }
    return host;
#else 
    HOSTENT* pHE = gethostbyaddr( (const s8 FAR*)&m_addr, 
                                  sizeof(m_addr), 
                                  AF_INET );
    if( NULL == pHE || NULL == pHE->h_name )
    {
        throw system_exception("IPAddress::getHostName(), gethostbyaddr failed");
    }
    return pHE->h_name;
#endif
}

IPAddress IPAddress::getByName( const std::string& host )
{
#ifndef WIN32
    struct addrinfo hints, *res;
    memset( &hints, 0, sizeof(hints) );

    hints.ai_flags  = AI_CANONNAME;
    hints.ai_family = AF_INET;

    int rv = getaddrinfo(host.c_str(), NULL, &hints, &res);
    if( 0 != rv )
    {
        s8 buf[ERROR_MSG_SIZE];
        sprintf(buf, "IPAddress::getByName(\"%s\") failed: %s", host.c_str(), gai_strerror(rv) );
        throw system_exception( buf, errno );  
    }

    IPAddress addr = ((sockaddr_in*) res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    return addr;
#else 
    hostent *pHE = NULL;

    u32 addr = inet_addr( host.c_str() );
    if( INADDR_NONE != addr )
    {   /* contains an (Ipv4) Internet Protocol dotted address ? */
        in_addr rv;
        memcpy( &rv, &addr, sizeof(rv) );
        return rv;
    }
    else
    {	  
		pHE = gethostbyname( host.c_str() );	
        if( NULL == pHE )
        {
            throw system_exception("gethostbyname", SOCKET_ERRNO );
        }
    }	  
    return *((in_addr*) pHE->h_addr_list[0]);
#endif
}

IPAddress IPAddress::getLocalHost()
{
    s8 buf[MAXHOSTNAMELEN];
    buf[0] = 0;

    if( 0 != gethostname(buf, MAXHOSTNAMELEN) )
    {
        throw system_exception("gethostname", errno);
    }
    return getByName(buf);
}

std::string IPAddress::getLocalHostName()
{
    s8 buf[MAXHOSTNAMELEN];
    buf[0] = 0;

    if( 0 != gethostname(buf, MAXHOSTNAMELEN) )
    {
        throw system_exception("gethostname", errno);
    }
    return buf;
}

bool IPAddress::operator==( const IPAddress& aRhs ) const
{
    return (0 == ::memcmp(&m_addr, &aRhs.m_addr, sizeof(m_addr)));
}

bool IPAddress::operator!=( const IPAddress& aRhs ) const
{
    return !(*this == aRhs);
}

IPAddress& IPAddress::operator=( const struct in_addr& addr )
{
    m_addr = addr;
    return *this;
}

void IPAddress::init()
{
#ifdef WIN32
    assert( FD_SETSIZE >= REQUIRED_FD_SETSIZE );
	
    WSADATA wsaData;	
    WORD wVersionRequested = MAKEWORD( 2, 0 );
    int err = WSAStartup( wVersionRequested, &wsaData );
    if( err != 0 ) 
    {
        throw system_exception("WSAStartup, we could not find a usable WinSock DLL.");
    }
#endif 

#ifndef WIN32
    /*  It is possible to receive SIGPIPE signal on UNIX systems while
        working with network subsystem. By default program will be terminated.
        The only possible way to avoid program crash is to ignore such signal.
    */

    struct sigaction act;

    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if( sigaction(SIGPIPE, &act, NULL) == -1 )
        throw system_exception("Can not ignore SIGPIPE signal.");
#endif
}

void IPAddress::cleanup()
{
#ifdef WIN32
	WSACleanup( );
#else
    /* # error not supported OS */
#endif 
}
