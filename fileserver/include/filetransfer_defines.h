#ifndef __file_transfer_defines_h__
#define __file_transfer_defines_h__

#include "refcounted.h"
#include <map>

#define DEF_RECONNECT_INTERVAL  8000
#define DEF_SENDING_INTERVAL    0
#define DEF_PACKAGE_SIZE        60000
#define DEF_RECVBUFFER_SIZE     65535 /* the maximum value of window size. */

#define TAG_START_CONTENT       ("<Hello. You must create the new file ")
#define TAG_CONTENT_SIZE        ("<Size of file is ")
#define TAG_FINISH_CONTENT      ("<Bye! You must close a file descriptor/>")

/////////////////////////////////////////////////////////////
// Forward classes declaration
class TCPSockClient;
class File;
class Task;

/////////////////////////////////////////////////////////////
// Task type enumeration and types
enum TaskSpec {
    connect_TaskSpec = 1,
    send_TaskSpec = 2,
    recv_TaskSpec = 3,
};

// Sockets objects container (key is socket fd)
typedef std::map<u32,RefCountedPtr<TCPSockClient>> Fd2SocketT;

// File objects container (key is socket fd)
typedef std::map<u32,std::auto_ptr<File>> Fd2FileT;

//////////////////////////////////////////////////////////////
// Creator for asyncronious actions
class TaskFactory
{
public:
    virtual Task* create_task(TaskSpec type, TCPSockClient* conn) = 0;
    virtual void destroy_task(Task* task) = 0;
    virtual void newlink_task(TaskSpec type, TCPSockClient* conn) = 0;
};

#endif /* __file_transfer_defines_h__ */
