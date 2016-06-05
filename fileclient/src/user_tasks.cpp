#include "user_tasks.h"
#include "notify_base.h"

using namespace std;

/****************************************************************/
ConnectionTask::ConnectionTask(const std::string& name,
                               const IPAddress& addr, 
                               u16 port,
                               TaskFactory* factory,
                               NotifyBase* notifyMgr,
                               TCPSockClient* connection)
    : Task(name),
    addr_(addr),
    port_(port),
    factory_(factory),
    notifyMgr_(notifyMgr)
{
    if( connection )
        connection_.reset(connection);
}

ConnectionTask::~ConnectionTask()
{
}

void ConnectionTask::run()
{
    MGuard g( lock_ );

    std::string host_name = addr_.getHostAddress();

    TCPSockClient* s = connection_.get();
    if( s && !s->is_open() ) // reconnecting
    {
        try {
            connection_->connect(addr_, port_);
        }
        catch(const Exception& ex) {
            notifyMgr_->warning(get_name() + " - WARNING: " + ex.what());
            notifyMgr_->debug(get_name() + " - WARNING: " + ex.what());
        }
        factory_->destroy_task(this);
    }
    else if( s == NULL ) // first connect
    { 
        std::string except_txt;
        notifyMgr_->notify("Connecting to FileServer...\n");
        try {
            s = new TCPSockClient(addr_, port_);
        }
        catch( const Exception& ex ) {
            except_txt = ex.what();
        }

        if( s == NULL ) {
            string msg = get_name() + " - WARNING: Failed to connect to FileServer on " + host_name + ":" + 
                 tostring(port_) + " - " + except_txt;
            notifyMgr_->warning( msg );
            notifyMgr_->debug( msg );
            return;
        }
    }

    connection_.reset(s);
    connection_->add_ref();
    factory_->newlink_task(connect_TaskSpec,s);
    g.release();

    string msg = get_name() + " - NOTE: We have connection #" + tostring(s->get_fd()) + " with FileServer on " + 
                     host_name + ":" + tostring(port_);
    notifyMgr_->notify(msg);
    notifyMgr_->debug(msg);

    // set connection alive
    factory_->destroy_task(this);
}

/**************************************************************/
SendingTask::SendingTask( const std::string& name,
                          File* sendingFile,
                          TaskFactory* factory,
                          NotifyBase* notifyMgr,
                          TCPSockClient* connection,
                          u32 packages_size)
    : Task(name),
    sendingFile_(sendingFile),
    factory_(factory),
    notifyMgr_(notifyMgr),
    shutdown_(false),
    packages_size_(packages_size)
{
    connection_.reset( connection );
}

SendingTask::~SendingTask()
{
    MGuard g( lock_ );
    shutdown_ = true;
}


void SendingTask::run()
{
    MGuard g( lock_ );

    if( connection_.get() && !connection_->is_open() ) {
        notifyMgr_->debug( get_name() + " - WARNING: session was closed. Kill me, please!" );
        notifyMgr_->warning( get_name() + " - WARNING: session was closed. Kill me, please!" );
        factory_->destroy_task( this );
        return;
    }

    u8* buf  = NULL;
    i32 read = 0;
    string tag_inside;

    static bool filePartSent = false;
    // sending last tag
    if( sendingFile_->eof() && filePartSent ) 
    {
        filePartSent = false;
        tag_inside = TAG_FINISH_CONTENT;
        connection_->send(tag_inside.c_str(), tag_inside.length()+1);

        string msg = get_name() + " - INFO: \"" + sendingFile_->path() + 
            "\" is sucesfully sent to host " + connection_->getTarget();
        notifyMgr_->notify(msg);
        // stop the sending tasks chain
        return;
    }

    // sending first tag
    if( !filePartSent && (filePartSent = true))
    {
        string newfile = sendingFile_->path();

        tag_inside = TAG_START_CONTENT;
        tag_inside += newfile + "/>";
        tag_inside += TAG_CONTENT_SIZE;
        tag_inside += tostring(sendingFile_->size()) + "/>";
    }

    // sending on any case
    buf = new u8[packages_size_+tag_inside.length()+1];
    if( !tag_inside.empty() )
        memcpy(buf, tag_inside.c_str(), tag_inside.length() );
    try {
        read = fread(buf+tag_inside.length(), 1, packages_size_, sendingFile_->handle());
    }
    catch(...){}
    g.release();

    string exc;
    if( read > 0 )
    {
        try {
            i32 write = read + tag_inside.length();
            read = connection_->send(buf, write);
            if( read > 0 ){
                notifyMgr_->debug( get_name() + " - NOTE: sent " + tostring(read) + " bytes.");
                notifyMgr_->notify( get_name() + " - NOTE: sent " + tostring(read) + " bytes.");
            }
            else if( write > 0 )
                sendingFile_->seek( -write, SEEK_CUR );
        }
        catch(const Exception& ex) {
            exc = get_name() + " - ERROR: " + ex.what();
        }
    }

    if( buf )
        delete[] buf;

    if( exc.empty() ) {
        // activate the next sending tasks
        Task* task = factory_->create_task(send_TaskSpec, connection_.get());
        if( task == NULL )
            connection_.abandon();
    }
    else {
        notifyMgr_->debug( exc );
        notifyMgr_->error( exc );
    }
}
