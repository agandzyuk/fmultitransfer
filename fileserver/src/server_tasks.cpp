#include "server_tasks.h"
#include "notify_base.h"

using namespace std;

/**************************************************************/
RecvTask::RecvTask( const std::string& name,
                    TaskFactory* factory,
                    NotifyBase* notifyMgr,
                    TCPSockClient* connection,
                    File* recvFile)
    : Task(name),
    factory_(factory),
    notifyMgr_(notifyMgr),
    recvFile_(recvFile),
    connection_(connection),
    receiver_(connection, notifyMgr),
    shutdown_(false)
{
    connection_->add_ref();
}

RecvTask::~RecvTask()
{
    MGuard g( lock_ );
    shutdown_ = true;
}

void RecvTask::run()
{
    MGuard g( lock_ );
    if( shutdown_ ) return;

    if( connection_.get() && !connection_->is_open() ) {
        notifyMgr_->debug( get_name() + " - WARNING: session was closed. Kill me, please!" );
        notifyMgr_->warning( get_name() + " - WARNING: session was closed. Kill me, please!" );
        factory_->destroy_task( this );
        return;
    }

    RawMessagesT messages;
    
    string exmsg;
    try {
        bool done = false;
        if( 0 < receiver_.receive( BufferParser(0, recvFile_), &messages, &done) )
        {
            for(RawMessagesT::const_iterator It = messages.begin(); 
                It != messages.end(); ++It)
            {
                recvFile_->write(It->get(), It->size(), true);
            }
        }

        if( done )
            recvFile_->close();
    }
    catch(const Exception& ex) {
        exmsg = get_name() + " - ERROR: " + ex.reason();
        notifyMgr_->error(exmsg);
        notifyMgr_->debug(exmsg);
    }

    if( exmsg.empty() ) 
    {
        // end of file received, so we set some delay for unblocked recv
        if( messages.empty() )
            Thread::sleep(200);

        // activate the next recv tasks
        Task* task = factory_->create_task(recv_TaskSpec, connection_.get());
        if( task == NULL )
            connection_.abandon();
    }
    else 
    {
        notifyMgr_->debug( get_name() + " - WARNING: has exception, so we suspend the connection.");
        notifyMgr_->warning( get_name() + " - WARNING: has exception, so we suspend the connection.");
    }
}
