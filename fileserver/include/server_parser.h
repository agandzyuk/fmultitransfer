#ifndef __server_parser_h__
#define __server_parser_h__

#include "filetransfer_defines.h"
#include "message.h"
#include "mutex.h"

////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Message> RawMessagesT;

class BufferParser
{
public:
    /*  Parser splits the data from connection on 'splitBy' parts if it is nonzero */
    BufferParser(u16 splitBy, File* recvFile);

    /*  Search for the packages in the buffer.
        @param packages - container where recevied packages will be located (only one in current implementation)
        @param done - end of file is received
        @Returns the number of packages.
    */
    u16 operator()(const u8* buffer,
                   u32 bufferSize,
                   RawMessagesT* packages,
                   bool* done) const;

    /* Auxiliary classes that represens execeptions that takes place during 
       handling of the garbled or zero messages.
    */
    class GarbledMsgReceivedException : public Exception {
    public: GarbledMsgReceivedException(const std::string& aReason) : Exception(aReason) {}
        inline const std::string& reason() const { return m_reason; }
    };
    class ZeroMsgReceivedException : public Exception {
    public: ZeroMsgReceivedException(const std::string& aReason) : Exception(aReason) {}
        inline const std::string& reason() const { return m_reason; }
    };

protected:
    /*  Parse start tags in incoming buffer
        @Returns the number of parsed bytes in start tag or -1 when buffer is empty
    */
    u16 parseStartTags(const s8* buffer, u32 bufferSize, std::string* path, u32* sizeOfFile) const;

private:
    u16   splitBy_;
    File* recvFile_;
};

////////////////////////////////////////////////////////////////////////////////
class NotifyBase;

class BufferReceiver 
{
public:
    BufferReceiver(TCPSockClient* connection, NotifyBase* notifyMgr);
    ~BufferReceiver();

    int receive(const BufferParser& parser, RawMessagesT* messages, bool* done);
    void clear();

private:
    Mutex lock_;            /* protect buffer */
    Message buffer_;        /* contains the data received earlier (if any) */
    NotifyBase* notifyMgr_; /* notification manager */
    RefCountedPtr<TCPSockClient> connection_; /* client connection */
};

#endif /* __server_parser_h__ */
