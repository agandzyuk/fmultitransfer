#include "dispatcher.h"
#include "server_tasks.h"

using namespace std;

/////////////////////////////////////////////////////////////////////
Dispatcher::Dispatcher(u16 serverPort, const IPAddress& serverHost)
    : Thread("FileServer"),
    shutdown_(false)
{
    IPAddress::init();

    server_.reset( new FileServer(serverPort, serverHost, this, this) );
    start();

    string msg = "\nFileServer is started on \"" + serverHost.getHostName() + ":" + tostring(serverPort) + "\"";
    msg += "\n\tpress'Q' or 'Esc' to FileServer exit";
    notify(msg);
    debug(msg);
}

Dispatcher::~Dispatcher()
{
    if( !shutdown_ )
        shutdown();
}

void Dispatcher::shutdown()
{
    runner_.cancel();
    runner_.join();

    MGuard guard( lock_ );
    shutdown_ = true;

    server_->stop();
    server_->join();
    fd2file_.clear();

    cancel();
}

void Dispatcher::run()
{
    int ch;
    do
    {
        fflush( stdin );
        ch = getch();
        ch = toupper(ch);
        switch( ch )
        {
        case 'Q':
        case SC_CTRLC:
        case SC_CTRLZ:
        case SC_ESC:
            ch = 'Q';
            break;
        case 0:
            break;
        default:
            break;
        }
        {
            MGuard guard( lock_ );
            if( shutdown_  ) ch = 'Q';
        }
        fflush(stdin);
    }
    while( ch != 'Q' && ch != SC_ESC );
    sleep(1000);

    shutdown();
}

Task* Dispatcher::create_task(TaskSpec type, TCPSockClient* conn)
{
    if( type == recv_TaskSpec )
    {
        u32 fd = conn->get_fd();
        if( fd2file_.end() == fd2file_.find(fd) )
            fd2file_.insert( Fd2FileT::value_type(fd,new File()) );

        RecvTask* task = new RecvTask("recvtask-" + tostring(fd),
                                      this, this, conn,
                                      fd2file_[fd].get());
        runner_.schedule(task,0,0);
        return task;
    }
    return NULL;
}

void Dispatcher::destroy_task( Task* task )
{
    runner_.cancel(task);
}

