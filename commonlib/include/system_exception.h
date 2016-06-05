#ifndef __system_exception_h__
#define __system_exception_h__

#include "system_defines.h"

#ifndef WIN32
#include <errno.h>
/* Make ERRNO equals to standart C errno */
#define ERRNO (errno)
#else 
/* Make ERRNO equals to Win32 GetLastError() */
#define ERRNO (GetLastError())
#endif /* #ifndef WIN32 */


#include "generic_exception.h"

/**
 * System exception.
 */
class system_exception : public Exception
{
public:
    /**
     * Constructs the system exception with the given errno.
     */
    system_exception(const std::string& msg, i32 errNo = ERRNO);

    /** Return the system-depended error code. */
    i32 getErrNo() const { return errNo_;}

private:
    i32 errNo_;
};

/**
 * Would block socket exception.
 */
class WouldBlockException:public system_exception
{
public:
    /**
     * Constructs the WouldBlock exception with the given description.
     */
    WouldBlockException(const std::string &descr);
};

#endif /* __system_exception_h__*/
