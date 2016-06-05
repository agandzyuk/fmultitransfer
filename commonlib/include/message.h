#ifndef __message_h__
#define __message_h__

#include "common_types.h"
#include "generic_exception.h"
#include "useful.h"

#include <cstdlib>
#include <cassert>
#include <memory>
#include <vector>
#include <queue>


/**********************************************************/
class Message 
{
public:
    Message() 
        : buffer_(NULL), size_(0), allocated_(0), protected_(false)
    {}

    Message( u32 bufSize ) 
        : buffer_(NULL), allocated_(0), protected_(false)
    {
        if( bufSize > 0 )
            this->reserve(bufSize);
        else
            size_ = 0;
    }

    Message( const u8* srcBuf, u32 bufSize ) 
        : buffer_(NULL), size_(0), allocated_(0), protected_(false)
    {
        this->set(srcBuf, bufSize);
    }

    Message( const Message& msg ) 
        : buffer_(NULL), size_(0), allocated_(0), protected_(false)
    {
        *this = msg;
    }

    virtual ~Message() 
    {
        if( !protected_ )
            clear();
    }

    inline Message& operator=(const Message& msg);
    inline u8* get(u32* size);
    inline const u8* get(u32* size) const;
    inline const u8* get( void ) const;
    inline u8* get( void );
    /* copy buffers */
    inline void set(const u8* srcBuf, u32 bufSize);
    inline void add(const u8* srcBuf, u32 bufSize);
    inline void reserve(u32 bufSize);
    inline u32 size() const;
    inline void clear();
    inline void erase(u32 bytes);
    inline void resize(u32 bufSize);

protected:
    inline void set_protected_using(u32 bytes);

    u8* buffer_;
    u32 size_;

private:
    u32  allocated_;
    bool protected_;
};
typedef std::vector<Message> MessagesT;

inline Message& Message::operator=(const Message& msg) {
    if( protected_ ) {
        assert( !"Message::operator= Trying to copy the data on protected region!" );
        throw Exception("Message exception: trying to copy the data on protected region");
    }

    reserve( msg.size_ );
    set(msg.buffer_, msg.size_);
    return *this;
}

inline u8* Message::get(u32* size) { 
    *size = size_;
    return buffer_; 
}

inline const u8* Message::get(u32* size) const { 
    *size = size_;
    return buffer_; 
}

inline const u8* Message::get() const { 
    return buffer_; 
}

inline u8* Message::get() { 
    return buffer_; 
}

inline u32 Message::size() const
{ return size_; }

inline void Message::set(const u8* srcBuf, u32 bufSize) {
    if( srcBuf == NULL && bufSize == 0 ) {
        assert( !"Message::set Trying to set the <null> data!" );
        throw Exception("Message exception: trying to set the <null> data");
    }
    if( protected_ ) {
        assert( !"Message::set Trying to set a data over the protected allocation!" );
        throw Exception("Message exception: trying to set a data over the protected allocation");
    }

    if( buffer_ == NULL ) {
        buffer_ = (u8*)malloc(bufSize);
        size_ = bufSize;
        allocated_ = size_;
    }
    else if ( bufSize > allocated_ ) {
        buffer_ = (u8*)realloc(buffer_, bufSize);
        size_ = bufSize;
        allocated_ = size_;
    }
    else if ( bufSize < allocated_ ) {
        size_ = bufSize;
    }
    memcpy(buffer_, srcBuf, size_);
}

inline void Message::add(const u8* srcBuf, u32 bufSize)
{
    if( srcBuf == NULL && bufSize == 0 ) {
        assert( !"Message::add Trying to add the fragment with <null> data!" );
        throw Exception("Message exception: trying to add the fragment with <null> data");
    }

    if( protected_ ) {
        assert( !"Message::add Trying to add a data to the protected allocation!" );
        throw Exception("Message exception: trying to add a data to the protected allocation");
    }

    if( buffer_ == NULL ) {
        set(srcBuf, bufSize);
        return;
    }
    
    if( (size_ + bufSize) > allocated_ ) {
        buffer_ = (u8*)realloc(buffer_, size_ + bufSize );
        allocated_ = size_ + bufSize;
    }
        
    u8* ptr = buffer_ + size_;
    size_ += bufSize;
    memcpy(ptr, srcBuf, bufSize);
}


inline void Message::reserve(u32 bufSize) {
    if( protected_ ) {
        assert( !"Message::reserve Trying to reserve the data amount on protected allocation!" );
        throw Exception("Message exception: trying to reserve the data amount on protected allocation");
    }

    if( buffer_ == NULL ) {
        buffer_ = (u8*)malloc(bufSize);
        size_ = bufSize;
        allocated_ = size_;
        return;
    }

    if( bufSize == 0 ) {
        clear();
    }
    else if( bufSize > size_ )
    {
        buffer_ = (u8*)realloc(buffer_, bufSize);
        size_ = bufSize;
        allocated_ = size_;
    }
    else if( bufSize < size_ )
        size_ = bufSize;
}

inline void Message::resize(u32 bufSize)
{
    if( protected_ ) {
        assert( !"Message::resize Trying to resize the protected allocation!" );
        throw Exception("Message exception: trying to resize the protected allocation");
    }

    if( bufSize > allocated_ )
    {
        buffer_ = (u8*)realloc(buffer_, bufSize);
        size_ = bufSize;
        allocated_ = size_;
    }

    size_ = bufSize;
}

inline void Message::clear() 
{
    if( protected_ ) {
        assert( !"Message::clear Trying to clear the protected allocation!" );
        throw Exception("Message exception: trying to clear the protected allocation");
    }

    assert( !(allocated_ && !buffer_) );
    assert( !(!allocated_ && buffer_) );
    if( buffer_ ) {
        free(buffer_);
    }
    buffer_ = NULL;
    size_ = 0;
    allocated_ = 0;
}

inline void Message::erase( u32 bytes ) {
    if( protected_ ) {
        assert( !"Message::erase Trying to erase data on the protected allocation!" );
        throw Exception("Message exception: trying to erase data on the protected allocation");
    }

    if( bytes <= size_ ) {
        memmove(buffer_, buffer_+bytes, size_-bytes);
        size_ -= bytes;
    }
    else {
        assert( !"Message::erase Åhe length of the controlled sequence is less than the number of bytes to erase!" );
        throw Exception("Message erase: the length of the controlled sequence (" + tostring(size_) + 
            ") is less than the number of bytes to erase (" + tostring(bytes) + ")");
    }
}

inline void Message::set_protected_using(u32 bytes)
{
    if( bytes == 0 ) {
        protected_ = false;
        return;
    }

    if( size_ == 0 || size_ > bytes )
        size_ = bytes;
    allocated_ = bytes;
    protected_ = true;
}

/**********************************************************/
struct MemBlock 
{
    std::auto_ptr<u8> block_;

    MemBlock()
    {}

    MemBlock(u32 size) 
    {
        block_.reset( (u8*)malloc( size ) );
    }

    MemBlock(const MemBlock& memblock) 
    {
        block_.reset( const_cast<MemBlock*>(&memblock)->block_.release() );
    }

    MemBlock& operator=(const MemBlock& memblock) 
    {
        block_.reset( const_cast<MemBlock*>(&memblock)->block_.release() );
        return *this;
    }
};

/**************************************************************************/
struct MemAllocTester
{
    std::vector< MemBlock > testblocks_;

    MemAllocTester(u64 testsize)
    {
        u64 tested_ = 0;
        testblocks_.resize( (u32)(testsize/0x80000000) + 1 );
        do {
            if( testsize - tested_ > 0x80000000 )
            {
                testblocks_.push_back( MemBlock( 0x80000000 ) );
                tested_ += 0x80000000;
            }
            else 
            {
                testblocks_.push_back( MemBlock( (u32)(testsize-tested_) ) );
                tested_ += testsize - tested_;
            }
        }
        while( tested_ < testsize );
    }

};

/**/
#endif /* __message_h__ */
