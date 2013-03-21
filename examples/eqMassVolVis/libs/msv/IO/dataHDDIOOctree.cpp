
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataHDDIOOctree.h"

#include <msv/util/str.h>  // toString
#include <msv/util/fileIO.h>  // fileSize
#include <msv/util/debug.h>

#include <msv/util/fileIO.h>

//#include <boost/interprocess/file_mapping.hpp>
//#include <boost/interprocess/mapped_region.hpp>

#include <assert.h>
#include <fstream>

#include <msv/tree/volumeTreeBase.h>

//using namespace boost::interprocess;

namespace massVolVis
{


DataHDDIOOctree::DataHDDIOOctree( const VolumeFileInfo& fileInfo )
    : DataHDDIO( fileInfo )
    , _baseName( "/octree_file_" )
{
}


constVolumeTreeBaseSPtr DataHDDIOOctree::getTree() const
{
    VolumeTreeBaseSPtr tree = VolumeTreeBaseSPtr( new VolumeTreeBase( getSourceDims(), getBlockDim() ));
    tree->loadHistograms( getHistorgamFileName() );

    return tree;
}


namespace
{
/**
 *  Returns a ^2 number of blocks that fits in one file, given the file size is 2 GB.
 *  id of the octree can be shifted left by that number to get the file number.
 */
uint32_t _pow2NumberOfBlocks( uint32_t size )
{
    size >>= 1;

    int shift = 0;
    while( size > 0 )
    {
        size >>= 1;
        shift++;
    }
    return 31 - shift; // 2 GB per file max
}
}// namespace


/**
 *  Name should contain directory of the octree
 */
void DataHDDIOOctree::_getFileNameAndOffset( const uint32_t     id,
                                                   std::string& name, uint32_t&  offset )
{
    const uint32_t bs         = getBlockSize_();
    const uint32_t pow2Blocks = _pow2NumberOfBlocks( bs );

    const uint32_t fileNum = id >> pow2Blocks;

    offset = ( id - ( fileNum << pow2Blocks )) * bs;

    name.append( _baseName );
    name.append( strUtil::toString( fileNum ));
    name.append( ".raw" );
}


bool DataHDDIOOctree::read(  const uint32_t id, void* dst )
{
    assert( isBlockSizeValid() );
    assert( id != 0 );

          std::string fName = getDataFileDir();
          uint32_t    offset;
    const uint32_t    blockSize = getBlockSize_();
    _getFileNameAndOffset( id-1, fName, offset );

    util::InFile inFile;
    if( inFile.open( fName, std::ios::binary, true ) &&
        inFile.read( offset, blockSize, dst ))
        return true;

    return false;

/*
// mapp file and read from it
//    LOG_INFO << "trying to read from: " << fName.c_str() << std::endl;
    file_mapping mappedFile( fName.c_str(), read_only );
    mapped_region mappedRegion( mappedFile, read_only, offset, blockSize );

    if( mappedRegion.get_size() != blockSize )
    {
        LOG_ERROR << "Size of the mapped region doesn't match the size of the data" << std::endl;
        return false;
    }

    memcpy( dst, mappedRegion.get_address(), blockSize );
*/
}


bool DataHDDIOOctree::allocateAllFiles( const uint32_t maxId )
{
    if( maxId < 1 )
    {
        LOG_INFO << "Nothing to allocate " << std::endl;
        return true;
    }

    const uint32_t bs      = getBlockSize_();
    const uint64_t nBlocks = 1 << _pow2NumberOfBlocks( bs );
    if( nBlocks < 1 )
    {
        LOG_ERROR << "Number of blocks per file can't be 0!" << std::endl;
        return false;
    }

    std::vector<byte> dVec( bs );
    if( dVec.size() != bs )
    {
        LOG_ERROR << "Can't allocate " << bs<< " bytes" << std::endl;
        return false;
    }
    byte* data = &dVec[0];

    // create/extend the last file 
    write( maxId, data );

    // create/extend all other files
    uint64_t currentId = nBlocks;
    while( currentId < maxId )
    {
        write( currentId, data );
        currentId += nBlocks;
    }

    return true;
}


bool DataHDDIOOctree::write(  const uint32_t id, const void* src )
{
    assert( isBlockSizeValid() );

          std::string fName = getDataFileDir();
          uint32_t    offset;
    const uint32_t    blockSize = getBlockSize_();
    _getFileNameAndOffset( id-1, fName, offset );

// Create / extend a file if necessary
    if( util::fileSize( fName ) < offset+blockSize )
    {
        std::filebuf fbuf;
        if( !fbuf.open( fName.c_str(), std::ios_base::ate | std::ios_base::out | std::ios_base::binary ))
        {
            LOG_ERROR << "Can't open file for writing: " << fName.c_str() << std::endl;
        }
        // Set the size
        fbuf.pubseekoff( offset+blockSize-1, std::ios_base::beg );
        fbuf.sputc(0);
        LOG_INFO << "file " << fName.c_str() << " extended file up to: " << offset+blockSize << std::endl;
    }

/*
// mapp file and write to it
    file_mapping mappedFile( fName.c_str(), read_write );
    mapped_region mappedRegion( mappedFile, read_write, offset, blockSize );

    if( mappedRegion.get_size() != blockSize )
    {
        LOG_ERROR << "Size of the mapped region doesn't match the size of the data"  << std::endl;
        return false;
    }

    memcpy( mappedRegion.get_address(), src, blockSize );
*/

    std::ofstream os;
    os.open( fName.c_str(), std::ios_base::out | std::ios_base::in | std::ios_base::binary  );
    if( !os.is_open( ))
    {
        LOG_ERROR << "Can't open file to write: " << fName.c_str() << std::endl;
        return false;
    }

    os.seekp( offset, std::ios::beg );
    if( os.tellp() != offset )
    {
        LOG_ERROR << "Can't proceed to the offset: " << offset << " to write file: " << fName.c_str() << std::endl;
        return false;
    }

    os.write( static_cast<const char*>(src), blockSize );

    if( os.fail( ))
        LOG_ERROR << "Some error happen during writing to a file: " << fName.c_str() 
                  << " with the offset: " << offset 
                  << " of " << blockSize << " bytes." << std::endl;

    os.close();

    return true;
}


}




















