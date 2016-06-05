#ifndef __fileserver_h__
#define __fileserver_h__

#include "filetransfer_defines.h"
#include "tcpserver.h"
#include "thread.h"

//////////////////////////////////////////////////////////////////////////////////
class NotifyBase;

class FileServer: public Thread, public TCPSockServer
{
public:
    FileServer(u16 port, 
               const IPAddress& host, 
               TaskFactory* factory, 
               NotifyBase* notifyMgr);
    ~FileServer();
    void stop();

protected:
    void run();

private:
    Mutex   lock_;              /* Safes an asynchronical stop in FileServer owner thread */
    bool    running_;           /* The flag for FileServe lifecycle */

    Fd2SocketT   fd2socket_;    /* Linkage socket descriptor to connection object */
    TaskFactory* factory_;      /* Factory for recv tasks creation */
    NotifyBase*  notifyMgr_;    /* Notifications object */
};

#endif /* __fileserver_h__ */
