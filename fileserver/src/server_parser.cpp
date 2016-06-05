#include "server_parser.h"
#include "dispatcher.h"
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////////////////
BufferParser::BufferParser(u16 splitBy, File* recvFile)
    : splitBy_(splitBy),
    recvFile_(recvFile)
{}

u16 BufferParser::operator()(const u8* buffer,
                             u32 bufferSize,
                             RawMessagesT* messages,
                             bool* done) const
{
    static u32 commonsize = 0;
    *done = false;

    if( bufferSize == 0 )
        throw ZeroMsgReceivedException("buffer size is 0");
    if( buffer == NULL )
        throw ZeroMsgReceivedException("buffer is <null>");

    if( !recvFile_->isOpened() )
    {
        commonsize = 0;

        string path;
        u32 sizeOfFile = 0;
        u16 firstTagBytes = parseStartTags((const s8*)buffer, bufferSize, &path, &sizeOfFile);
        buffer += firstTagBytes;
        bufferSize -= firstTagBytes;
        
        string::size_type pos = path.find_last_of("\\/");
        if( pos != string::npos && pos < path.length() )
            path = path.substr(pos+1);

        recvFile_->open(path,"wb+");
        if( !recvFile_->isOpened() )
            throw Exception("\nCan't open file \"" + path + "\" for writing");
        else
            cout << "Receiving file \"" + path + "\"...\n";

        recvFile_->resize(sizeOfFile);
    }

    int decimal = commonsize;
    // delete console numbers
    do{ decimal /= 10; cout << "\r";} while(decimal > 10);
    // delete console " bytes" word 
    do{ cout << "\r"; decimal++; } while(decimal < 6);

    commonsize += bufferSize;
    cout << commonsize << " bytes";

    if( commonsize > recvFile_->size() ) 
    {
        const u8* eop = buffer;
        u16 finishTagLen = strlen(TAG_FINISH_CONTENT);
        if( bufferSize < finishTagLen )
            throw GarbledMsgReceivedException("Finish tag is not found in the received buffer from sock ");

        bufferSize -= finishTagLen+1;

        if( memcmp(buffer+bufferSize, TAG_FINISH_CONTENT,finishTagLen) )
            throw GarbledMsgReceivedException("Finish tag is not found in the received buffer from sock ");

        cout << "\nFile transfering \"" + recvFile_->path() + "\" is done.\n\n";
        *done = true;
        commonsize = 0;
    }

    if( splitBy_ ) {
        u16 num = (u16)(bufferSize / splitBy_);
        for(u16 i = 0; i < num; ++i) {
            messages->push_back( Message(buffer+splitBy_*i, splitBy_) );
        }
        return num;
    }

    if( bufferSize ) {
        messages->push_back( Message(buffer, bufferSize) );
        return 1;
    }
    return 0;
}

u16 BufferParser::parseStartTags(const s8* buffer, u32 bufferSize, std::string* path, u32* sizeOfFile) const
{
    const char* ptr = buffer;
    u32 startTagLen = strlen(TAG_START_CONTENT);

    if( bufferSize < startTagLen+2 )
        throw GarbledMsgReceivedException("received buffer doesn't contain a filepath: length " + 
                                          tostring(bufferSize) + " bytes");

    if( 0 != memcmp(ptr, TAG_START_CONTENT, startTagLen) )
        throw GarbledMsgReceivedException("received buffer doesn't contain a filepath: invalid content \"" + 
                                           string().assign(ptr, startTagLen) + "\"");

    ptr += startTagLen;
    const char* eop = strstr(ptr, "/>");
    if( eop == NULL )
        throw GarbledMsgReceivedException("transfering file has no path");

    path->assign(ptr, eop-ptr);
    ptr = (eop+=2);

    startTagLen = strlen(TAG_CONTENT_SIZE);
    if( bufferSize < strlen(TAG_START_CONTENT)+startTagLen+4 )
        throw GarbledMsgReceivedException("received buffer doesn't contain the information with file size:"
                                          " length " + tostring(bufferSize) + " bytes");

    if( 0 != memcmp(ptr, TAG_CONTENT_SIZE, startTagLen) )
        throw GarbledMsgReceivedException("received buffer doesn't contain the information with file size:"
                                          " invalid content \"" + string().assign(ptr, startTagLen) + "\"");
    string strSizeOfFile;
    ptr += startTagLen;
    if( (NULL == (eop = strstr(ptr, "/>"))) || 
        (0 == (strSizeOfFile = string(ptr,eop-ptr)).length()) )
    {
        throw GarbledMsgReceivedException("transfering file has no info size");
    }
    eop += 2;
    
    *sizeOfFile = atol(strSizeOfFile.c_str());
    return (eop-buffer);
}

/////////////////////////////////////////////////////////////////////////
BufferReceiver::BufferReceiver(TCPSockClient* connection, NotifyBase* notifyMgr)
    : connection_(connection),
    notifyMgr_(notifyMgr)
{
    connection_->add_ref();
    connection_->set_nonblocking(true);
}

BufferReceiver::~BufferReceiver()
{
    MGuard g(lock_);
    clear();
}

void BufferReceiver::clear()
{
    MGuard g(lock_);
    if( 0 < buffer_.size() )
    {
        buffer_.clear();
    }
}

int BufferReceiver::receive(const BufferParser& parser, RawMessagesT* messages, bool* done)
{
    MGuard g(lock_);

    i32 sz = buffer_.size();
    buffer_.reserve( sz + DEF_RECVBUFFER_SIZE );

    u8* ptr = buffer_.get() + sz;
    i32 nReceived = 0;
    nReceived = connection_->recv(ptr, DEF_RECVBUFFER_SIZE);
    if( 0 == nReceived ) {
        buffer_.clear();
        throw Exception("Connection is down (EOF recevied)");
    }
    else if( -1 == nReceived ) {
        buffer_.resize( sz );
        return -1;
    }

    buffer_.resize(nReceived + sz);

    try {
        const u8* ptr = buffer_.get();
        if( 0 == parser(ptr, buffer_.size(), messages, done) )
            return -1;

        nReceived = 0;
        RawMessagesT::iterator It = messages->begin();
        for(; It != messages->end(); ++It )
            nReceived += It->size();
        clear();
    }
    catch(const BufferParser::GarbledMsgReceivedException& ex) {
        string msg = "ERROR: Garbled buffer received (" + ex.reason() + ")";
        notifyMgr_->error(msg);
        notifyMgr_->debug(msg);
        buffer_.clear();
        return -1;
    }
    catch(const BufferParser::ZeroMsgReceivedException& ex)
    {
        string msg = "WARN: Zero buffer received (" + ex.reason() + ")";
        notifyMgr_->warning(msg);
        notifyMgr_->debug(msg);
        buffer_.clear();
        return -1;
    }

    assert(nReceived >= 0);
    return nReceived;
}
