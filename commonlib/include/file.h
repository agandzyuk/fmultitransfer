#ifndef __file_h__
#define __file_h__

#include "system_exception.h"
#include <string>

 /* An representation of file or directory. */
class File
{
public:
    typedef FILE* handle_t;

    /*  Checks if the given file already exists.   
        @return true if so, otherwise false.
    */
    static bool doesExist( const std::string& path );

    /*  Creates new empty file */
    static void createEmpty( const std::string& path );

    /*  Returns current directory path */
    static std::string getCurrentDirectory( void );

    File();
    File( const std::string& path, const std::string& openmode );
    virtual ~File();

    /*  Supporting for non-destructive File objects */
    void open( const std::string& path, const std::string& openmode );
    void close( void );

    /* Input/Output */
    u32 read( void* buf, u32 size );
    u32 write( const void* buf, u32 size, bool flush = false );
        
    /*  Returns the file size.  */
	i64 size( void ) const;

    /*  Resizes file */
    void resize( i64 size );

    /*  Standard file io routines */
    void flush( void );
    void rewind( void );
    i64  tell( void ) const;
    void seek( i64 offset, i32 origin );
    bool eof( void ) const;

    bool isOpened( void ) const;

    handle_t handle( void ) const
    { return handle_; }

    const std::string& path() const 
    { return path_; }

private:
    handle_t handle_;
    std::string path_;
};

#endif /* __file_h__ */
