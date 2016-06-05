#include "fileserver.h"
#include "dispatcher.h"
#include "notify_base.h"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
//  Application main
int main()
{
    IPAddress::init();
    u16 listenPort = 0;
    try 
    {
        cout << "\nPlease specify the server listen port: ";
        cin  >> listenPort;
        if( listenPort == 0 )
            listenPort = 80;

        IPAddress local = IPAddress::getLocalHost();
        Dispatcher dispatcher(listenPort, local);
        dispatcher.join();
    }
    catch(const Exception& ex)
    {
#ifdef WIN32
        char oembuf[512] = {0};
        CharToOemBuff( ex.reason().c_str(), (LPSTR)oembuf, strlen( ex.reason().c_str() )+1 );
        cout << ">> Error: " << oembuf;
#else
        cout << ">> Error: " << ex.reason();
#endif
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// FileServer class implementation
FileServer::FileServer(u16 port, 
                       const IPAddress& host,
                       TaskFactory* factory,
                       NotifyBase* notifyMgr)
    : Thread("FileServer"), 
    TCPSockServer(port, &host, true),
    factory_(factory),
    notifyMgr_(notifyMgr),
    running_(true)
{
    start();
}

FileServer::~FileServer()
{
    if( running_ )
        stop();
}

void FileServer::stop()
{
    notifyMgr_->debug("FileServer work is finishing...");

    // safe section
    {
        MGuard g( lock_ );
        running_ = false;
        TCPSockServer::close();
    }

    // waiting for thread normal exit
    join();
    fd2socket_.clear();

    string msg = "FileServer stopped!";
    notifyMgr_->notify(msg);
    notifyMgr_->debug(msg);
}

void FileServer::run()
{
    TCPSockClient* connection;
    bool safeRunning = true;

    try {
        while( safeRunning && (connection = accept()) )
        {
            // new incoming connection
            fd2socket_.insert(Fd2SocketT::value_type(connection->get_fd(),connection));
            notifyMgr_->notify( "INFO: new  incoming connection from \"" + connection->getIPAddress().getHostName() + "\"");

            // start data receiving
            if( NULL == factory_->create_task(recv_TaskSpec, connection) ) {
                notifyMgr_->error( "ERROR: FileServer unexpected stopping: can't read data from connection");
                MGuard g( lock_ );
                running_ = false;
            }
            
            // safe section to stopping check 
            MGuard g( lock_ );
            safeRunning = running_;
        }
    }
    catch(...) // ignore socket exception (accept throws when server is closing)
    {}
    running_ = false;
}
