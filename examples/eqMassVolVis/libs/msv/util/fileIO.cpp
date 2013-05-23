
#include "fileIO.h"

#include "debug.h"

#include <msv/util/str.h>
#include <msv/util/debug.h>

#include <sys/stat.h>
#include <unistd.h> // access()

namespace util
{

bool extendFile( const std::string& fileName, int64_t size )
{
    std::filebuf fbuf;
    if( !fbuf.open( fileName.c_str(), std::ios_base::ate | std::ios_base::out | std::ios_base::binary ))
    {
        LOG_ERROR << "Can't open file for writing: " << fileName.c_str() << std::endl;
        return false;
    }
    if( size < 1 )
        return true;
    // Set the size
    fbuf.pubseekoff( size-1, std::ios_base::beg );
    if( fbuf.sputc(0) != EOF )
        return true;
    return false;
}


bool fileOrDirExists( const std::string& path )
{
    if( access( path.c_str(), 0 ) == 0 )
        return true;

    return false;
}


namespace
{
bool _isDir( const std::string& path )
{
    struct stat status;
    stat( path.c_str(), &status );

    return (status.st_mode & S_IFDIR);
}
}


bool isDir( const std::string& path )
{
    if( !fileOrDirExists( path ))
        return false;

    return _isDir( path );
}


bool isFile( const std::string& path )
{
    if( !fileOrDirExists( path ))
        return false;

    return !_isDir( path );
}


std::string getDirName( const std::string& path )
{
    if( isDir( path ))
        return path;

    size_t pos = path.find_last_of( '/' );
    if( pos == std::string::npos )
        return "./";

    return path.substr( 0, pos+1 );
}


void paddFilename( std::string& name, uint32_t value, uint32_t maxVal )
{
    uint32_t padd = value > 0 ? value : 1;
    while( padd < maxVal )
    {
        name.append( "0" );
        padd *= 10;
    }
    name.append( strUtil::toString( value ));
}


uint64_t fileSize( const std::string& fileName )
{
    std::ifstream f;
    f.open( fileName.c_str(), std::ios_base::binary | std::ios_base::in );

    if( !f.good() || f.eof() || !f.is_open( ))
        return 0;

    f.seekg( 0, std::ios_base::beg );
    std::ifstream::pos_type begin_pos = f.tellg();
    f.seekg( 0, std::ios_base::end );

    return static_cast<uint64_t>( f.tellg() - begin_pos );
}


void copyFile( const std::string& src, const std::string& dst )
{
    std::ifstream s( src.c_str(), std::ios::binary );
    std::ofstream d( dst.c_str(), std::ios::binary );

    if( !s.is_open() )  throw std::string( "can't open file to read ")  + src;
    if( !d.is_open() )  throw std::string( "can't open file to write ") + dst;

    d << s.rdbuf();
    s.close();
    d.close();
}


bool InFile::open( const std::string& fileName, const std::ios_base::openmode mode, bool safe )
{
    _fileName = 0;
    _tmpName.clear();
    _is.close();

    _is.open( fileName.c_str(), std::ios_base::in | mode );
    if( _is.is_open( ))
    {
        if( safe )
            _fileName = fileName.c_str();
        else
        {
            _tmpName = fileName;
            _fileName = _tmpName.c_str();
        }
        _offset = 0;
        return true;
    }

    LOG_ERROR << "Can't open file to read: " << fileName.c_str() << std::endl;
    return false;
}


bool InFile::read( int64_t offset, uint32_t size, void* dst )
{
    if( !_is.is_open())
    {
        LOG_ERROR << "Can't read since file is not open" << std::endl;
        return false;
    }

    _is.seekg( offset, std::ios::beg );
    if( _is.tellg() != offset )
    {
        LOG_ERROR << "Can't proceed to the offset: " << offset << " to read file: " << _fileName << std::endl;
        return false;
    }

    if( size == 0 )
        return true;

    _is.read( static_cast<char*>(dst), size );

    if( _is.gcount() != size || _is.fail( ))
    {
        LOG_ERROR << "Some error happen during reading of a file: " << _fileName
                  << " with the offset: " << offset
                  << " of " << size << " bytes." << std::endl;
        return false;
    }

    _offset += size;
    return true;
}


bool InFile::read( uint32_t size, void *dst )
{
    if( !_is.is_open())
    {
        LOG_ERROR << "Can't read since file is not open" << std::endl;
        return false;
    }

    if( size == 0 )
        return true;

    _is.read( static_cast<char*>(dst), size );

    if( _is.fail( ))
    {
        LOG_ERROR << "Some error happen during reading of a file: " << _fileName
                  << " with the offset: " << _offset
                  << " of " << size << " bytes." << std::endl;
        return false;
    }

    _offset += size;
    return true;
}


bool InFile::read( const std::string& fileName, const std::ios_base::openmode mode, int64_t offset, uint32_t size, void *dst )
{
    InFile file;
    if( !file.open( fileName, mode, true ))
        return false;

    return file.read( offset, size, dst );
}


bool OutFile::open( const std::string& fileName, const std::ios_base::openmode mode, bool safe )
{
    _fileName = 0;
    _tmpName.clear();
    _os.close();

    _os.open( fileName.c_str(), std::ios_base::in | mode );
    if( _os.is_open( ))
    {
        if( safe )
            _fileName = fileName.c_str();
        else
        {
            _tmpName = fileName;
            _fileName = _tmpName.c_str();
        }
        _offset = 0;
        return true;
    }

    LOG_ERROR << "Can't open file to write: " << fileName.c_str() << std::endl;
    return false;
}


bool OutFile::write( int64_t offset, uint32_t size, const void* src )
{
    if( !_os.is_open())
    {
        LOG_ERROR << "Can't write since file is not open" << std::endl;
        return false;
    }

    _os.seekp( offset, std::ios::beg );
    if( _os.tellp() != offset )
    {
        LOG_ERROR << "Can't proceed to the offset: " << offset << " to write file: " << _fileName << std::endl;
        return false;
    }

    if( size == 0 )
        return true;

    _os.write( static_cast<const char*>(src), size );

    if( _os.fail( ))
    {
        LOG_ERROR << "Some error happen during writing to a file: " << _fileName
                  << " with the offset: " << offset
                  << " of " << size << " bytes." << std::endl;
        return false;
    }

    _offset += size;
    return true;
}


bool OutFile::write( uint32_t size, const void *src )
{
    if( !_os.is_open())
    {
        LOG_ERROR << "Can't write since file is not open" << std::endl;
        return false;
    }

    if( size == 0 )
        return true;

    _os.write( static_cast<const char*>(src), size );

    if( _os.fail( ))
    {
        LOG_ERROR << "Some error happen during writing to a file: " << _fileName
                  << " with the offset: " << _offset
                  << " of " << size << " bytes." << std::endl;
        return false;
    }

    _offset += size;
    return true;
}

bool OutFile::write( const std::string& fileName, const std::ios_base::openmode mode, int64_t offset, uint32_t size, const void *dst )
{
    OutFile file;
    if( !file.open( fileName, mode, true ))
        return false;

    return file.write( offset, size, dst );
}


}
