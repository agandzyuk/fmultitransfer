#ifndef __dispatcher_h__
#define __dispatcher_h__

#include "fileserver.h"
#include "timer.h"
#include "task.h"
#include "file.h"
#include "notify_base.h"

#include <iostream>

//////////////////////////////////////////////////////////////
// Dispatcher thread class
class Dispatcher : public Thread,
                   public TaskFactory, 
                   public NotifyBase
{
public:
    Dispatcher(u16 serverPort, const IPAddress& serverHost);
    ~Dispatcher();

    void shutdown();

    /* TaskFactory implementation */
    Task* create_task(TaskSpec type, TCPSockClient* conn);
    void destroy_task(Task* task);
    /* no need for server */
    void newlink_task(TaskSpec type, TCPSockClient* conn) {}

protected:
    virtual void run();

    /*  Notifies the common info 
        @param  aNotification - warning message
     */
    virtual void notify(const std::string& aNotification)
    { 
#ifdef WIN32
        char oembuf[512];
        *oembuf = 0;
        CharToOemBuff( aNotification.c_str(), oembuf, aNotification.length()+1);
        *const_cast<std::string*>(&aNotification) = oembuf;
#endif
        std::cout << aNotification << std::endl; 
    }

    /*  Notifies about the warning
        @param  aNotification - warning message
     */
    virtual void warning(const std::string& aNotification)
    { 
#ifdef WIN32
        char oembuf[512];
        *oembuf = 0;
        CharToOemBuff( aNotification.c_str(), oembuf, aNotification.length()+1);
        *const_cast<std::string*>(&aNotification) = oembuf;
#endif
        std::cout << aNotification << std::endl; 
    }

    /*  Notifies about the error
        @param  aNotification - error message
     */
    virtual void error(const std::string& aNotification)
    { 
#ifdef WIN32
        char oembuf[512];
        *oembuf = 0;
        CharToOemBuff( aNotification.c_str(), oembuf, aNotification.length()+1);
        *const_cast<std::string*>(&aNotification) = oembuf;
#endif
        std::cout << aNotification << std::endl;
    }

    /*  Notifies debug information
        @param  aNotification - debug message
     */
    virtual void debug(const std::string& aNotification)
    {
        std::auto_ptr<File> debug_file;
        try {
            debug_file.reset( new File("./server.log", "a+") );
        }
        catch( const Exception& ex)
        {
            std::string except = "Can't open file ./server.log: ";
#ifdef WIN32
            char oembuf[512];
            *oembuf = 0;
            CharToOemBuff( ex.what(), oembuf, aNotification.length()+1);
            except += oembuf;
#endif
            std::cout << except << std::endl;
        }
        
        debug_file->write( aNotification.c_str(), aNotification.length() );
        debug_file->write( "\n", 1 );
    }

private:
    std::auto_ptr<FileServer> server_;
    Timer runner_;      /* Recv tasks async executor
                           Running the each task are subsequent in separate thread
                        */

    bool     shutdown_; /* The flag for dispatcher stopping */
    Mutex    lock_;     /* For safe stopping of dispatcher owner thread */
    Fd2FileT fd2file_;  /* Linkage connection to choosen file */
};

#endif /* __dispatcher_h__  */
