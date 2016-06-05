#include "mainframe.h"
#include "user_tasks.h"

using namespace std;

/////////////////////////////////////////////////////////////////////
namespace {
    // IP4 address form validation 
    string check_IP( const string& str_ip )
    {
        string wellformed;
        StringsT strs;
        if( 4 == split( str_ip, '.', &strs ) )
        {
            StringsT::iterator It = strs.begin();
            for( ; It != strs.end(); ++It )
            {
                u16 i = atoi(It->c_str());
                if ( i < 0 || i > 255 )
                    break;
                wellformed += (It == strs.begin() ? "" : ".") + tostring(i);
            }
            if( It == strs.end() )
            {
                return wellformed;
            }
        }
        cout << "Incorrect IP address. Please retype: ";
        return "";
    }
}

Mainframe::Mainframe()
    : Thread("Mainframe"), 
    shutdown_(false),
    reconnect_interval_(DEF_RECONNECT_INTERVAL),
    send_interval_(DEF_SENDING_INTERVAL),
    packages_size_(DEF_PACKAGE_SIZE)
{
    IPAddress::init();
    start();
}

Mainframe::~Mainframe()
{
    shutdown();
}

void Mainframe::shutdown()
{
    timer_.cancel();
    timer_.join();

    MGuard guard( lock_ );
    shutdown_ = true;
    cancel();
}

void Mainframe::run()
{

    int ch;
    char tmp[100];
    string server_ip;
    do
    {
        fflush( stdin );
        ch = getch();
        ch = toupper(ch);
        switch( ch )
        {
        case 'N':
            do {
                set_silence_logging(true);

                cout << "You choosen a new connection creation.\n"
                        "Please type the server IP address: ";
                do {
                    *tmp = 0;
                    scanf("%s", tmp);
                    tmp[15] = 0;
                }
                while( (server_ip = check_IP(tmp)).empty() );

                i32 port;
                cout << "Please type port number: ";
                scanf("%d", &port );
                if( port == 0 || port > 65534 )
                {
                    cout << "Invalid port number\n...request canceled\n";
                    break;
                }

                ConnectionTask* task = new ConnectionTask(string("connection-") + server_ip,
                                                          IPAddress::getByName(server_ip),
                                                          port, this, this, (TCPSockClient*)NULL);
                timer_.schedule(task, 50, reconnect_interval_);
            } while(false);
            set_silence_logging(false);
            break;
        case 'X':
            do {
                set_silence_logging(true);

                cout << "You choosen connection stopping. Please type the target IP address: ";
                do {
                    *tmp = 0;
                    scanf("%s", tmp);
                    tmp[15] = 0;
                }
                while( (server_ip = check_IP( server_ip)).empty() );

                timer_.cancel(string("connection") + "-" + server_ip);
                Fd2SocketT::iterator It = fd2sockets_.begin();
                for(; It != fd2sockets_.end(); ++It)
                    if( It->second->getTarget() == server_ip )
                        break;

                if( It == fd2sockets_.end() ) {
                    cout << "We have not already connected or disconnected connections to " 
                         << server_ip << "\n...reconnection stopped\n";
                    break;
                }
                
                u32 fd = (u32)It->second->get_fd();
                timer_.cancel("sendtask" + tostring(fd));
                if( It->second->is_open() ) {
                    It->second->close();
                    cout << "Connection with " << server_ip << " is disconnected.\n";
                }
                else {
                    cout << "Connection to " << server_ip << " is not connected and only reconnection process has stopped.\n" 
                           "You can restore connection only through creation by command 'N'.\n";
                }
            } while(false);
            set_silence_logging(false);
            break;
        case 'R':
            do {
                set_silence_logging(true);

                cout << "You choosen connection restoring by id. Connection id to restore is ";
                u32 id = 0;
                scanf("%d", &id);

                Fd2SocketT::iterator It = fd2sockets_.find(id);
                if( It == fd2sockets_.end() ) {
                    cout << "We have not already connected or disconnected connections with id " 
                         << id << "\n...request canceled\n";
                    break;
                }
                if( It->second->is_open() ) {
                    cout << "Connection " << id << " id already exists and keep a link with " 
                        << It->second->getIPAddress().getHostName() << "\n...request canceled\n";
                    break;
                }

                ConnectionTask* task = new ConnectionTask(string("connection-") + It->second->getTarget(),
                                                          It->second->getIPAddress(),
                                                          It->second->get_port(),
                                                          this, this, NULL);
                timer_.schedule( task, 50, reconnect_interval_ );
            } while(false);
            set_silence_logging(false);
            break;
        case 'I':
            do {
                set_silence_logging(true);

                cout << "\nCurrent reconnecting time interval is " 
                     << reconnect_interval_/1000 << 
                     " seconds.\nSet the new reconnecting time interval <enter>?\n";

                fflush(stdin); ch = getch();
                if( ch == SC_ENTER ) {
                    printf("(No greater than 1 hour) New reconnecting time interval (seconds) is ");
                    u32 tme = 0; scanf("%d",&tme);
                    if( tme < 3600 ) {
                        reconnect_interval_ = tme * 1000; printf("OK\n"); ch = 0; 
                        break;
                    }
                }
                printf("...request canceled\n");
                ch = ch == 3 ? 'Q' : 0;
            } while(false);
            set_silence_logging(false);
            break;
        case 'S':
            do {
                set_silence_logging(true);
                cout << "\nCurrent sending time interval is " <<  send_interval_ << " milliseconds.\n"
                        "Set the new sending time interval <enter>?\n";
                fflush(stdin); ch = getch();
                if( ch == SC_ENTER ) {
                    cout << "New sending time interval (milliseconds) is ";
                    u32 tme = 0; scanf("%d",&tme);
                    if( tme < 3600 ) {
                        send_interval_ = tme; 
                        cout << "OK\n"; 
                        ch = 0;
                        break;
                    }
                }
                cout << "...request canceled\n";
                ch = ch == 3 ? 'Q' : 0;
            } while(false);
            set_silence_logging(false); 
            break;
        case 'P':
            do {
                set_silence_logging(true);
                cout << "\nCurrent sending package size is " << packages_size_ << " bytes.\n"
                        "Set the new sending package size <enter>?\n";
                fflush(stdin); ch = getch();
                if( ch == SC_ENTER ) {
                    cout << "New package size (bytes) is ";
                    u32 sz = 0; scanf("%d",&sz);
                    packages_size_ = sz; 
                    cout << "OK\n"; 
                    ch = 0;
                    break;
                }
                cout << "...request canceled\n";
                ch = ch == 3 ? 'Q' : 0;
            } while(false);
            set_silence_logging(false); 
            break;
        case 'F':
            do
            {
                set_silence_logging(true);

                cout << "You choosen an assigning the file to replay into connection with id.\n"
                        "Please enter connection id: ";

                u32 id = 0;
                scanf("%d", &id);
                Fd2SocketT::iterator It = fd2sockets_.find(id);
                for(; It != fd2sockets_.end(); ++It)
                    if ( (u32)It->second->get_fd() == id )
                        break;
                if( It == fd2sockets_.end() )
                {
                    cout << "We have not already connected or disconnected connections with id " 
                         << id << "\n...request canceled\n";
                    break;
                }

                cout << "Now enter the path to replaying file: ";
                char buf[1024] = {0}; scanf("%s",buf);
                if( strlen(buf) && !File::doesExist(buf) ) {
                    cout << "Incorrect path was entered and file not exists.\n"
                           "...request canceled\n";
                    break;
                }

                if( fd2file_.end() == fd2file_.find(id) )
                    fd2file_.insert(Fd2FileT::value_type(id,(File*)NULL));

                fd2file_[id].reset(new File(buf, "rb"));
                cout << "\"" << buf << "\" is opened for reading.\n"
                        "Sending will be stopped after the entire content be sent.\n";

                create_task(send_TaskSpec, It->second.get());
            } while(false);
            set_silence_logging(false);
            break;
        case 'M':
            set_silence_logging(true);
            print_menu();
            set_silence_logging(false);
            break;
        case 'Q':
        case SC_CTRLC:
        case SC_CTRLZ:
        case SC_ESC:
            ch = 'Q';
            break;
        case 0:
            break;
        default:
            {
                char txt[2] = {0}; *txt = ch; 
                cout << "no command " << txt << endl;
            }
            break;
        }
        {
            MGuard guard( lock_ );
            if( shutdown_  ) ch = 'Q';
        }
        fflush(stdin);
    }
    while( ch != 'Q' && ch != SC_ESC );
    cout << "\n...exiting FileClient\n";
    sleep(1000);
}

Task* Mainframe::create_task(TaskSpec type, TCPSockClient* conn)
{
    static bool canceled = false;
    static TCPSockClient* spActiveConnection = NULL;

    if( type == send_TaskSpec )
    {
        if( NULL == (spActiveConnection = conn))
            return NULL;

        u32 fd = conn->get_fd();
        SendingTask* task = new SendingTask("sendtask-" + tostring(fd),
                                            fd2file_[fd].get(), 
                                            this, this, 
                                            spActiveConnection,
                                            packages_size_);
        try {
            timer_.schedule(task, send_interval_, 0);
        }
        catch(...) {
            delete task;
            return NULL;
        }
        return task;
    }
    else if( type == connect_TaskSpec )
    {
        ConnectionTask* task = new ConnectionTask("connection-" + conn->get_fd(),
                                                  conn->getIPAddress(), conn->get_port(),
                                                  this, this, conn);
        timer_.schedule(task, 50, reconnect_interval_); /* pause to delete previous copy of task */
        return task;
    }
    return NULL;
}

void Mainframe::destroy_task( Task* task )
{
    timer_.cancel(task);
}

void Mainframe::newlink_task(TaskSpec type, TCPSockClient* conn)
{
    if( type == connect_TaskSpec )
        fd2sockets_.insert(Fd2SocketT::value_type(conn->get_fd(),conn));
}