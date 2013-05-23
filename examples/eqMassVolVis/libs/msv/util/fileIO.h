
#ifndef UTIL_FILE_IO_H
#define UTIL_FILE_IO_H

#include <msv/types/types.h>
#include <fstream>


namespace util
{

void paddFilename( std::string& name, uint32_t value, uint32_t maxVal = 100000 );

uint64_t fileSize( const std::string& fileName );

bool extendFile( const std::string& fileName, int64_t size );

void copyFile( const std::string& src, const std::string& dst );

bool isDir(  const std::string& path );
bool isFile( const std::string& path );

bool fileOrDirExists( const std::string& path );

std::string getDirName( const std::string& path );


class InFile
{
public:
    InFile() : _fileName(0), _offset(0) {}
    ~InFile(){ _is.close(); _fileName = 0; }

    /**
     * filename is storred internally and it is used to print error messages. If it is
     * guaranted that original file name will not change while reading is performed then only
     * pointer to the string is saved (safe = true), otherwise file name is copied (safe = false).
     */
    bool open( const std::string& fileName, const std::ios_base::openmode mode, bool safe = false );
    bool read( int64_t offset, uint32_t size, void *dst );
    bool read( uint32_t size, void *dst );

    static bool read( const std::string& fileName, const std::ios_base::openmode mode, int64_t offset, uint32_t size, void *dst );
private:
    std::ifstream _is;
    std::string _tmpName;
    const char* _fileName;
    int64_t _offset;
};


class OutFile
{
public:
    OutFile() : _fileName(0), _offset(0) {}
    ~OutFile(){ _os.close(); _fileName = 0; }

    /**
     * filename is storred internally and it is used to print error messages. If it is
     * guaranted that original file name will not change while writing is performed then only
     * pointer to the string is saved (safe = true), otherwise file name is copied (safe = false).
     */
    bool open( const std::string& fileName, const std::ios_base::openmode mode, bool safe = false );
    bool write( int64_t offset, uint32_t size, const void *src );
    bool write( uint32_t size, const void *src );

    static bool write( const std::string& fileName, const std::ios_base::openmode mode, int64_t offset, uint32_t size, const void *dst );
private:
    std::ofstream _os;
    std::string _tmpName;
    const char* _fileName;
    int64_t _offset;
};

}

#endif //UTIL_FILE_IO_H
