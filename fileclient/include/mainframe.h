#ifndef __mainframe_h__
#define __mainframe_h__

#include "timer.h"
#include "task.h"
#include "notify_base.h"
#include "filetransfer_defines.h"
#include "file.h"

#include <iostream>

extern void print_menu();

//////////////////////////////////////////////////////////////
// Console menu class
class Mainframe : public Thread, 
                  public TaskFactory, 
                  public NotifyBase
{
public:
    Mainframe();
    ~Mainframe();

    void shutdown();

    /* TaskFactory implementation */
    Task* create_task(TaskSpec type, TCPSockClient* conn);
    void destroy_task(Task* task);
    void newlink_task(TaskSpec type, TCPSockClient* conn);


protected:
    virtual void run();

    /*  Notifies the common info 
        @param  aNotification - warning message
     */
    virtual void notify(const std::string& aNotification)
    { 
        if( silence_logging_ ) return;
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
        if( silence_logging_ ) return;
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
            debug_file.reset( new File("./client.log", "a+") );
        }
        catch( const Exception& ex)
        {
            std::string except = "Can't open file ./client.log: ";
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

    /*  Disables logging to console */
    void set_silence_logging(bool silence) {
        silence_logging_ = silence;
    }

private:
    Fd2SocketT  fd2sockets_; /* Linkage socket descriptor to connection object */
    Fd2FileT    fd2file_;    /* Linkage connection to choosen file */

    Timer timer_;
    u32 reconnect_interval_;
    u32 send_interval_;
    u32 packages_size_;

    bool silence_logging_;
    bool shutdown_; /* mainframe shutdown flag */
    Mutex lock_; /* mainframe safer lock */
};

#endif /* __mainframe_h__ */
