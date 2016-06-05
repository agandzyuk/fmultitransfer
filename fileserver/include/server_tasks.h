#ifndef __server_tasks_h__
#define __server_tasks_h__

#include "dispatcher.h"
#include "server_parser.h"

class BufferReceiver;

////////////////////////////////////////////////////////////////////////////
// Performs data receieving 
class RecvTask : public Task, public RefCounted
{
    friend class Dispatcher;
protected:
    RecvTask(const std::string& name,
             TaskFactory* factory,
             NotifyBase* notifyMgr,
             TCPSockClient* connection,
             File* recvFile);
    ~RecvTask();

    virtual void run();

private:
    Mutex lock_;
    bool shutdown_;

    File* recvFile_;
    TaskFactory* factory_;
    NotifyBase* notifyMgr_;
    RefCountedPtr<TCPSockClient> connection_;
    BufferReceiver receiver_;
};

#endif /* __server_tasks_h__ */
