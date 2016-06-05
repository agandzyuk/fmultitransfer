#ifndef __generic_exception_h__
#define __generic_exception_h__

#include <string>
#include <exception>

#include "common_types.h"
#include "system_defines.h"

class Exception : public std::exception 
{
public:
    Exception() {}
    Exception(const std::string& aReason)
#if defined(_MSC_VER) && defined(_DEBUG)
            : exception( aReason.c_str() )
#else
            : exception()
#endif
            , m_reason( aReason )
    {
#if defined (_DEBUG) && defined (WIN32)
        OutputDebugStringA(m_reason.c_str());
        OutputDebugStringA("\n");
#endif /* defined (_DEBUG) && defined (WIN32) */
    }

    /** Destructor. */
    virtual ~Exception() throw ()
    {
    }

    /**
    * Returns the reason for this exception.
    */
    virtual const s8 *what() const throw() 
    {
        return m_reason.c_str();
    }   

    /**
    * Returns the reason for this exception.
    */
    std::string const& reason() const throw() 
    {
        return m_reason;
    }   

protected:
    /* string that describes raised exception */
    std::string m_reason;  
};

/**
* Exception related to encryption. 
*/
class EncryptionException : public Exception
{
public:
        /* @param aMsg Message that explains exception. */
    EncryptionException(const std::string& aMsg) : Exception(aMsg){}
};

/**
* Invalid parameter Exception.
*/
class InvalidParameterException : public Exception
{
public:
    /* @param aMsg Message that explains exception. */
    InvalidParameterException(const std::string& aMsg) : Exception(aMsg){}
};

#endif /* __generic_exception_h__ */
