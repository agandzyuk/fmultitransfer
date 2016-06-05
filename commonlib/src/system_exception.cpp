#include <string>
#include <stdio.h>

#include "system_exception.h"

using namespace std;

const i32 BUFFER_SIZE = 512;

system_exception::system_exception(const std::string& aMsg, i32 aErrno)
    : errNo_(aErrno)
{
    s8 buf[BUFFER_SIZE];
    sprintf( buf, "%d", aErrno );

#ifndef WIN32
    m_reason = aMsg + " : " + strerror(aErrno) + " (" + buf + ")";
#else 
    HLOCAL hlocal = NULL; /* message buffer handle */
    BOOL fOk = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	                          NULL, aErrno, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                              (LPTSTR) &hlocal, 0, NULL );

    if( !fOk )
    {
	    /* Is it a network-related error? */
         HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);

         if( hDll != NULL ) 
         {
            fOk = FormatMessage(
               FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
               hDll, aErrno, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
               (PTSTR) &hlocal, 0, NULL);
            FreeLibrary(hDll);
         }
    }

    if( fOk && (NULL != hlocal) )
    {
	    if(fOk > 2 && ('\r' == static_cast<const s8*>(hlocal)[fOk-2]) )
        {
		    fOk -= 2;
	    }

	    m_reason = aMsg + " : " + string( (LPTSTR) hlocal, fOk) + " (" + buf + ")";
    }
    else
    {
	    m_reason = aMsg + " (Error code = " + buf + ")";
    }

    LocalFree(hlocal);
	
#endif /* WIN32 */
}
