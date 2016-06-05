#ifndef __ipaddress_h__
#define __ipaddress_h__

#include <string>

#ifndef WIN32
#include <netdb.h>
#endif 

#include "system_exception.h"
#include "useful.h"

/*  This class represents an Internet Protocol (IP) address.    */
class IPAddress
{
public:
    IPAddress();
    IPAddress( const struct in_addr& addr )
        : m_addr(addr)
    {}

    /*  Returns the IP address string "%d.%d.%d.%d".    */ 
    std::string getHostAddress() const;

    /*  Gets the host name for this IP address. 
        @throw system_exception
    */
    std::string getHostName() const;

    /*  Returns a low level system usable struct in_addr object.    */
    struct in_addr get_address() const 
    { return m_addr; }

    /*  Determines the IP address of a host, given the host's name. 
        The host name can either be a machine name, such as "www.yahoo.com", 
        or a string representing its IP address, such as "192.168.0.100".
        @throw system_exception
    */
    static IPAddress getByName(const std::string& host);

    /*  Returns the local host.
        @throw system_exception
    */
    static IPAddress getLocalHost();

    /*  Returns the local host name.
        @throw system_exception
    */
    static std::string getLocalHostName( void );

    bool operator==(const IPAddress& rhs) const;
    bool operator!=(const IPAddress& rhs) const;
    IPAddress& operator=(const struct in_addr& addr);

    /*  Inits all methods.
        Must be the first IPAddress method called by an application.
        @throw system_exception
    */
    static void init();

    /* Cleanups resources. */
    static void cleanup();

private:
    /* IP address */
    in_addr m_addr; 
};

#endif /* __ipaddress_h__ */
