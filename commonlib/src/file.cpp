#include "file.h"

#include <errno.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#   include <dirent.h>
#   include <unistd.h>
#else 
#   include <direct.h>
#   include <io.h>
#endif 

using namespace std;

bool File::doesExist(const std::string& path)
{
    return (access(path.c_str(), 0 ) != -1 );
}

void File::createEmpty( const std::string& path )
{
    std::filebuf fb;
    fb.open( path.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc );

    if( !fb.is_open() )
    {
        throw system_exception( "Cannot create new file: " + path );
    }
}

std::string File::getCurrentDirectory()
{
    s8 cCurrentPath[ 4096 ];
    s8* buf = NULL;

#ifdef WIN32
    if( NULL == (buf = _getcwd( cCurrentPath, sizeof(cCurrentPath) )) )
    {
        throw system_exception( "Cannot rerive current system path: _getcwd:", errno );
    }
#else
    if( NULL == (buf = getcwd(cCurrentPath, sizeof(cCurrentPath))) )
    {
        throw system_exception( "Cannot rerive current system path: getcwd:", errno );
    }
#endif

    cCurrentPath[ sizeof(cCurrentPath) - 1 ] = '\0'; /* not really required */
    return buf;
}

File::File() 
    : handle_(0)
{}

File::File( const std::string& path, 
            const std::string& openmode )
    : handle_(0)
{
    open( path, openmode );
}

File::~File()
{
    close();
}

void File::open( const std::string& path, const std::string& openmode )
{
    assert( !openmode.empty() );
    /*  trying to reopen */
    close();
    
#ifdef WIN32
    handle_ = fopen( path.c_str(), openmode.c_str() );
#else
    handle_ = fopen64( path.c_str(), openmode.c_str() );
#endif 
    if( 0 == handle_ )
		throw system_exception(std::string("Can't open file ") + path, ERRNO);
    path_ = path;
}

void File::close()
{
    if( isOpened() )
        fclose( handle_ );
    handle_ = 0;
}

u32 File::read( void* buf, u32 size )
{
    assert(nullptr != buf);
    u32 sz = fread(buf, 1, size, handle_);
    if( sz != size ) 
        throw system_exception("fread", ERRNO);
    return sz;
}

u32 File::write(const void* buf, u32 size, bool flushStream)
{
    assert(nullptr != buf);
    u32 sz = fwrite(buf, 1, size, handle_);
    if(/*0 != sz &&*/ sz != size)
        throw system_exception("fwrite", ERRNO);

    if (flushStream)   
        flush();
    return sz;
}

void File::flush()
{
    fflush( handle_ );
}

void File::rewind()
{
    ::rewind( handle_ );
}

i64 File::tell() const
{
#ifdef WIN32
    i64 res = _ftelli64( handle_ );
#else
    i64 res = ftello64( handle_ );
#endif
    if( -1LL != res )
        return res;                
    throw system_exception("_ftelli64");
}

void File::seek( i64 offset, i32 origin )
{
#ifdef WIN32
    if( 0 != _fseeki64( handle_, offset, origin ) )
        throw system_exception("_fseeki64");
#else
    if( 0 != fseeko64( handle_, offset, origin ) )
        throw system_exception("fseeko64");
#endif
}

bool File::eof() const
{
    return 0 != feof( handle_ );
}

bool File::isOpened() const
{
    return 0 != handle_;
}

i64 File::size() const
{
#ifdef WIN32
	return (i64)_filelength(_fileno( handle_ ));
#else
    struct stat64 fileInfo;
    memset(&fileInfo, 0, sizeof(fileInfo));
	fstat64(fileno( handle_ ), &fileInfo);
	return fileInfo.st_size;
#endif 
}

void File::resize( i64 size )
{
#ifdef WIN32
    HANDLE file = (HANDLE)_get_osfhandle( _fileno( handle_ ) );
    LARGE_INTEGER pos;
    pos.QuadPart = size;

    if( INVALID_SET_FILE_POINTER == SetFilePointer(file, pos.LowPart, &pos.HighPart, FILE_BEGIN) )
        throw system_exception( "SetFilePointer: " );

    if( 0 == SetEndOfFile( file ) )
        throw system_exception( "SetEndOfFile: " );
#else
	if (0 != ftruncate(fileno( handle_ ), size))
		throw system_exception("ftruncate: ");
#endif

    rewind();
}
