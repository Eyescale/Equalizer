
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataHDDIORaw.h"

#include <msv/util/fileIO.h>  // fileSize
#include <msv/util/debug.h>
#include <msv/util/fileIO.h>

#include <assert.h>
#include <fstream>
#include <cstring> // memcpy

namespace massVolVis
{

namespace
{
const int64_t _invalidZ = -1;
const int64_t _invalidD = -1;
}


DataHDDIORaw::DataHDDIORaw( const VolumeFileInfo& fileInfo )
    : DataHDDIO( fileInfo )
    , _oldZ( _invalidZ )
    , _oldD( _invalidD )
{

}


void DataHDDIORaw::setDataFileName( const std::string& name )
{
    DataHDDIO::setDataFileName( name );

    _oldZ = _invalidZ;
    _oldD = _invalidD;
}

namespace
{
void _copyData(       char*      dst,    // destination
                const char*      src,    // source
                const Vec3_ui32& dim,    // how much to copy
                const Vec3_ui32& dstDim, // destination size
                const Vec3_ui32& srcDim, // source size
                const byte       bytes ) // byts per value
{
    const uint32_t    lineSize =    dim.w * bytes;
    const uint32_t dstLineSize = dstDim.w * bytes;
    const uint32_t srcLineSize = srcDim.w * bytes;

    const uint32_t dstSliceSize = dstDim.w * dstDim.h * bytes;
    const uint32_t srcSliceSize = srcDim.w * srcDim.h * bytes;

    for( uint32_t z = 0; z < dim.d; ++z )
    {
              char* d = dst + dstSliceSize * z;
        const char* s = src + srcSliceSize * z;
        for( uint32_t y = 0; y < dim.h; ++y )
        {
            memcpy( d, s, lineSize );
            d += dstLineSize;
            s += srcLineSize;
        }
    }
}
}


bool DataHDDIORaw::read( const Box_i32& dim, void* dst )
{
    static bool errorReported = false;
    if( sizeof(std::streamoff) < sizeof(int64_t) && !errorReported )
    {
        LOG_ERROR << "sizeof(std::streamoff) < sizeof(int64_t): reading from large files is probably broken!" << std::endl;
        errorReported = true;
    }

    assert( isSourceSizeValid() );

    if( !dim.valid( ))
    {
        LOG_ERROR << "Input dimensions are invalid" << std::endl;
        return false;
    }

    const Vec3_ui32 srcDim = getSourceDims();
    const Vec3_ui32 dstDim = dim.getDim();

    // get actual coordinates
    Box_i32 bb( Vec3_ui32(0), srcDim );
    bb = bb.intersect( dim );

    const byte bytes = getBytesNum();
    if( bb != dim ) // our area is smaller than requested => clean initial data
        memset( dst, 0, dim.getAreaSize()*bytes );

    if( !bb.valid( ))
        return true;

    const Vec3_ui32 bbDim = bb.getDim();

    char* dstByte = reinterpret_cast<char*>( dst );

    // shift of the destination
    const uint64_t dstShift = ( uint64_t(dstDim.h)*dstDim.w*( bb.s.d-dim.s.d ) +
                                                   dstDim.w*( bb.s.h-dim.s.h ) +
                                                            ( bb.s.w-dim.s.w )  ) * uint64_t(bytes);
    dstByte += dstShift;

    // shift of the source
    const int64_t srcZ = int64_t(srcDim.h) * srcDim.w * bb.s.d * uint64_t(bytes);
    const int64_t srcY =                     srcDim.w * bb.s.h * uint64_t(bytes);
    const int64_t srcX =                                bb.s.w * uint64_t(bytes);

    // amout of data we are going to use
    const int64_t sliceSize = srcDim.h * srcDim.w * bbDim.d * bytes;

    if( srcZ != _oldZ || static_cast<int64_t>(bbDim.d) != _oldD )
    {
//        LOG_INFO << "  New depth to read from a raw file: " << srcZ << std::endl;

        const std::string& fName = getDataFileName();
        if( static_cast<int64_t>(util::fileSize( fName )) < srcZ+sliceSize )
        {
            LOG_ERROR << "File: " << fName.c_str() << " is too smll to read " << sliceSize 
                      << " bytes within the offset: " << srcZ << std::endl;
            return false;
        }

        util::InFile inFile;
        if( !inFile.open( fName, std::ios::binary, true ))
            return false;

        _tmp.resize( sliceSize );

        if( !inFile.read( srcZ, sliceSize, &_tmp[0] ))
            return false;

        _oldZ = srcZ;
        _oldD = static_cast<int64_t>(bbDim.d);
    }

    _copyData( dstByte, &_tmp[srcY+srcX], bbDim, dstDim, srcDim, bytes );

    return true;
}

/*

// ----------------------------------------------
//  Unbufferized reading
// ----------------------------------------------

namespace
{
void _copyData( char*            dst,    // destination
                      std::ifstream& is, // source
                const Vec3_ui32& dim,    // how much to copy
                const Vec3_ui32& dstDim, // destination size
                const Vec3_ui32& srcDim, // source size
                const byte       bytes ) // byts per value
{
    const uint32_t    lineSize =    dim.w * bytes;
    const uint32_t dstLineSize = dstDim.w * bytes;
    const uint32_t srcLineSize = srcDim.w * bytes;

    const uint32_t dstSliceSize = dstDim.w * dstDim.h * bytes;
    const uint32_t srcSliceSize = srcDim.w * srcDim.h * bytes;

    uint64_t pos = 0;
    for( uint32_t z = 0; z < dim.d; ++z )
    {
        char* d = dst + dstSliceSize * z;

        const uint64_t seekSize = srcSliceSize*z - pos;
        if( seekSize != 0 )
        {
            is.seekg( seekSize, std::ios_base::cur );
            pos = srcSliceSize*z;
        }
        for( uint32_t y = 0; y < dim.h; ++y )
        {
            is.read( d, lineSize );
            if( is.gcount() != lineSize )
            {
                LOG_ERROR << "Couldn't read enough from the file" << std::endl;
                return;
            }

            d += dstLineSize;

            if( srcLineSize - lineSize != 0 )
                is.seekg( srcLineSize-lineSize, std::ios_base::cur );
            pos += srcLineSize;
        }
    }
}
}

bool DataHDDIORaw::read( const Box_i32& dim, void* dst )
{
    static bool errorReported = false;
    if( sizeof(std::streamoff) < sizeof(int64_t) && !errorReported )
    {
        LOG_ERROR << "sizeof(std::streamoff) < sizeof(int64_t): reading from large files is probably broken!" << std::endl;
        errorReported = true;
    }

    assert( _isSizeValid() );

    if( !dim.valid( ))
    {
        LOG_ERROR << "Input dimensions are invalid" << std::endl;
        return false;
    }

    const Vec3_ui32 srcDim = getSize();
    const Vec3_ui32 dstDim = dim.getDim();

    // get actual coordinates
    Box_i32 bb( Vec3_ui32(0), srcDim );
    bb = bb.intersect( dim );

    const byte bytes = getBytesSize();
    if( bb != dim ) // our area is smaller than requested => clean initial data
        memset( dst, 0, dim.getAreaSize()*bytes );

    if( !bb.valid( ))
        return true;

    const Vec3_ui32 bbDim = bb.getDim();

    char* dstByte = reinterpret_cast<char*>( dst );

    // shift of the destination
    const uint64_t dstShift = ( uint64_t(dstDim.h)*dstDim.w*( bb.s.d-dim.s.d ) +
                                                   dstDim.w*( bb.s.h-dim.s.h ) +
                                                            ( bb.s.w-dim.s.w )  ) * uint64_t(bytes);
    dstByte += dstShift;

    // shift of the source
    const int64_t srcShift = ( uint64_t(srcDim.h)*srcDim.w* bb.s.d +            // Z
                                        srcDim.w * bb.s.h +                     // Y
                                                   bb.s.w ) * uint64_t(bytes);  // X

    // amout of data we are going to use
    const std::size_t srcSize = ( srcDim.h*srcDim.w* bbDim.d -
                                           srcDim.w* bb.s.h  -
                                                     bb.s.w   ) * bytes;

    const std::string& fName = getInName();
    if( fileSize( fName ) < srcShift+srcSize )
    {
        LOG_ERROR << "File: " << fName.c_str() << " is too smll to read " << srcSize 
                  << " bytes within the offset: " << srcSize << std::endl;
        return false;
    }

    std::ifstream is;
    is.open( fName.c_str(), std::ios_base::in | std::ios::binary );
    if( !is.is_open( ))
    {
        LOG_ERROR << "Can't open file to read: " << fName.c_str() << std::endl;
        return false;
    }

    is.seekg( srcShift, std::ios::beg );
    if( is.tellg() != srcShift )
    {
        LOG_ERROR << "Can't proceed to the offset: " << srcShift << " to read file: " << fName.c_str() << std::endl;
        is.close();
        return false;
    }

    _copyData( dstByte, is, bbDim, dstDim, srcDim, bytes );

    if( is.fail( ))
        LOG_ERROR << "Some error happen during reading of a file: " << fName.c_str() 
                  << " with the offset: " << srcShift 
                  << " of " << srcSize << " bytes." << std::endl;

    is.close();

    return true;
}
*/


/*

// ----------------------------------------------
//  Old memory mapped version of the file reding
// ----------------------------------------------


//#include <boost/interprocess/file_mapping.hpp>
//#include <boost/interprocess/mapped_region.hpp>

//using namespace boost::interprocess;

static void copyData(       byte*      dst,    // destination
                      const byte*      src,    // source
                      const Vec3_ui32& dim,    // how much to copy
                      const Vec3_ui32& dstDim, // destination size
                      const Vec3_ui32& srcDim, // source size
                      const byte       bytes ) // byts per value
{
    const uint32_t    lineSize =    dim.w * bytes;
    const uint32_t dstLineSize = dstDim.w * bytes;
    const uint32_t srcLineSize = srcDim.w * bytes;

    const uint32_t dstSliceSize = dstDim.w * dstDim.h * bytes;
    const uint32_t srcSliceSize = srcDim.w * srcDim.h * bytes;

    for( uint32_t z = 0; z < dim.d; ++z )
    {
              byte* d = dst + dstSliceSize * z;
        const byte* s = src + srcSliceSize * z;
        for( uint32_t y = 0; y < dim.h; ++y )
        {
            memcpy( d, s, lineSize );
            d += dstLineSize;
            s += srcLineSize;
        }
    }
}


bool DataHDDIORaw::read( const Box_i32& dim, void* dst )
{
    assert( _isSizeValid() );

    if( !dim.valid( ))
    {
        LOG_ERROR << "Input dimensions are invalid" << std::endl;
        return false;
    }

    const Vec3_ui32 srcDim = getSize();
    const Vec3_ui32 dstDim = dim.getDim();

    // get actual coordinates
    Box_i32 bb( Vec3_ui32(0), srcDim );
    bb = bb.intersect( dim );

    const byte bytes = getBytesSize();
    if( bb != dim ) // our area is smaller than requested => clean initial data
        memset( dst, 0, dim.getAreaSize()*bytes );

    if( !bb.valid( ))
        return true;

    const Vec3_ui32 bbDim = bb.getDim();

    byte* dstByte = reinterpret_cast<byte*>( dst );

    // shift of the destination
    const offset_t dstShift = ( offset_t(dstDim.h)*dstDim.w*( bb.s.d-dim.s.d ) +
                                                   dstDim.w*( bb.s.h-dim.s.h ) +
                                                            ( bb.s.w-dim.s.w )  ) * offset_t(bytes);
    dstByte += dstShift;

    // shift of the source
    const offset_t srcShift = ( offset_t(srcDim.h)*srcDim.w* bb.s.d +
                                         srcDim.w * bb.s.h +
                                                    bb.s.w ) * offset_t(bytes);

    // amout of data we are going to use
    const std::size_t srcSize = ( srcDim.h*srcDim.w* bbDim.d -
                                           srcDim.w* bb.s.h  -
                                                     bb.s.w   ) * bytes;

// mapp file and read from it
    file_mapping mappedFile( getInName().c_str(), read_only );
    mapped_region mappedRegion( mappedFile, read_only, srcShift, srcSize );

    if( mappedRegion.get_size() != srcSize )
    {
        LOG_ERROR << "Size of the mapped region doesn't match the size of the data"  << std::endl;
        return false;
    }

    const byte* srcByte = reinterpret_cast<byte*>( mappedRegion.get_address( ));

    copyData( dstByte, srcByte, bbDim, dstDim, srcDim, bytes );

    return true;
}
*/

}

