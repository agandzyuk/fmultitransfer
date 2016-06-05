#ifndef __user_tasks_h__
#define __user_tasks_h__

#include "mainframe.h"
#include "tcpclient.h"

class NotifyBase;

/* performs new connection or reconnects existant */
class ConnectionTask : public Task, public RefCounted
{
    friend class Mainframe;
protected:
    ConnectionTask(const std::string& name,
                   const IPAddress& addr, 
                   u16 port, 
                   TaskFactory* factory,
                   NotifyBase* notifyMgr,
                   TCPSockClient* pConnection);
    ~ConnectionTask();

    virtual void run();

private:
    Mutex     lock_;

    IPAddress addr_;
    u16       port_;
    TaskFactory* factory_;
    NotifyBase* notifyMgr_;
    RefCountedPtr<TCPSockClient> connection_;
};

/* performs sending */
class SendingTask : public Task, public RefCounted
{
    friend class Mainframe;
protected:
    SendingTask(const std::string& name,
                File* sendingFile,
                TaskFactory* factory,
                NotifyBase* notifyMgr,
                TCPSockClient* connection,
                u32 packages_size);
    ~SendingTask();

    virtual void run();

private:
    Mutex lock_;
    bool shutdown_;

    File* sendingFile_;
    TaskFactory* factory_;
    NotifyBase* notifyMgr_;
    RefCountedPtr<TCPSockClient> connection_;
    u32 packages_size_;
};

#endif /*__user_tasks_h__ */
