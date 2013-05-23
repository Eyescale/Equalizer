
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataConverter.h"

#include "dataManipulation.h"
#include <msv/tree/volumeTreeBase.h>
#include <msv/IO/dataHDDIOOctree.h>

#include <msv/util/debug.h>
#include <msv/util/hlp.h> // myClip
#include <msv/util/str.h>

#include <limits>
#include <cstring>
#include <assert.h>

namespace massVolVis
{

const std::string& DataConverter::getHelp()
{
    const static std::string help(
        std::string( "Simple model converter\n" ) +
        std::string( "\t Possible conversion directions:\n" ) +
        std::string( "\t\t raw     ( 8 bit) -> oct      ( 8 bit)\n" ) +
        std::string( "\t\t raw     (16 bit) -> oct      (16 bit)\n" ) +
        std::string( "\t\t raw_der ( 8 bit) -> oct      ( 8 bit)\n" ) +
        std::string( "\t\t oct     ( 8 bit) -> oct_head ( 8 bit)\n" ) +
        std::string( "\t\t oct     (16 bit) -> oct_head (16 bit)\n\n" ));

    return help;
}

void DataConverter::setDataReader( VolumeFileInfo* dataIn )
{
    _dataIn = dataIn;
    _hddIn  = dataIn->createDataHDDIO();
}

void DataConverter::setDataWriter( VolumeFileInfo* dataOut )
{
    _dataOut = dataOut;
    _hddOut  = _dataOut->createDataHDDIO();
}

// Converter TODO:
//  - move counter to a separate class, and clean counting code
//      (set message, scan number of blocks, update counter
//
//  - can iterate along Z axis with explicit block position specification
//      (don't find new last Z every time)

void DataConverter::convert()
{
    if( _dataIn == 0    || _dataOut == 0    ) FAIL( "_dataIn and _dataOut have to be set" );
    if( _inType == NONE || _outType == NONE ) FAIL( "_inType and _outType have to be set" );

// oct -> oct_head conversion
    if( _inType == OCT )
    {
        if( _outType != OCT_HEAD )
            FAIL( "it is not possible to convert \"oct\" to anything but \"oct_head\"" );
        if( _dataIn->getBlockSize_() != _dataOut->getBlockSize_( ))
            FAIL( "input and output dimensions have to be the same for this type of conversion" );

        std::vector<byte> src( _dataOut->getBlockSize_());

        try
        {
            _hddIn->read(   1, &src[0] );
            _hddOut->write( 1, &src[0] );
        }
        catch( char* e )
        {
            LOG_ERROR << "Error occured: " << e << std::endl;
        }
        return;
    }

// else
// raw -> oct conversion

    if( _inType != RAW && _inType != RAW_DER )
        FAIL( "input type is incorrect (it has to be \"raw\" or \"raw_der\"" );

    if( _outType != OCT )
        FAIL( "output type is incorrect (it has to be \"oct\")" );

    const byte borderDim = _dataOut->getBorderDim();

    VolumeTreeBase tree( _dataIn->getSourceDims(), _dataOut->getBlockDim() ); // out data is a cube

    const Vec3_ui32 dstBlockDim( _dataOut->getBlockDim() ); // original block dimensions
    const Vec3_ui32 dstBlockAndBordersDim( _dataOut->getBlockAndBordersDim() ); // block + borders

    const size_t dstBS = _dataOut->getBlockSize_();
    const size_t srcBS = ( _inType == RAW_DER ) ? dstBS*4 : dstBS; // max input block size

    std::vector<byte> src( srcBS );

// 1. read and store all original blocks

    // get the first child of the last level
    int64_t  pos      = 0;
    uint32_t blocks   = 1;
    uint64_t childPos = 0;
    while(( childPos = tree.getChild( pos )) < tree.getSize() )
    {
        pos = childPos;
        blocks *= 8;
    }
    if( pos+blocks != tree.getSize( ))
        FAIL( "incorrect position of last child" );

    LOG_INFO_ << "=======================" << std::endl;
    LOG_INFO  << "pos: " << pos << " blocks: " << blocks << std::endl;
    LOG_INFO  << "tree size: " << tree.getSize() << std::endl;

    LOG_INFO_ << "Allocating storage..." << std::endl;
    _hddOut->allocateAllFiles( tree.getNumberOfValidBlocks( ));
    LOG_INFO_ << "Done allocating storage." << std::endl;

    // get number of valid leaf blocks (used to count progress of execution)
    uint32_t validBlocks = 0;
    for( int64_t p = pos; p < pos+blocks; ++p )
    {
        const uint32_t nodeId = tree.getNodeId( p );
        if( nodeId != 0 )
            validBlocks++;
    }

    // copy original blocks. 
    // perform copying per z-ordered slice for better raw read performance
    uint32_t counter       =  0;
    int64_t  oldPercents   = -1;
    uint32_t blocksPercentage = hlpFuncs::myClip( validBlocks/100u, 100u, 1000u );

    int64_t maxZ = std::numeric_limits<int64_t>::max();
    while( true )
    {
        // find next last Z coordinate
        int64_t curPos  = -1;
        int64_t lastPos = -1;

        int64_t curMaxZ = tree.getAbsDataCoordinates( 0 ).s.z-1-borderDim;
        for( int64_t p = pos; p < pos+blocks; ++p )
        {
            const uint32_t nodeId = tree.getNodeId( p );
            if( nodeId == 0 )
                continue;

            const int64_t absCoordsZ = tree.getAbsDataCoordinates( p ).s.z-borderDim;
            if( absCoordsZ < maxZ )
            {
                if( absCoordsZ > curMaxZ )
                {
                    curPos  = p;
                    lastPos = p;
                    curMaxZ = absCoordsZ;
                }else
                {
                    if( absCoordsZ == curMaxZ )
                        lastPos = p;
                }
            }
        }
        maxZ = curMaxZ;

        if( curPos < 0 )
            break;

        // use next last Z coordinate to process original blocks in slabs
        for( int64_t p = curPos; p <= lastPos; ++p )
        {
            const uint32_t nodeId = tree.getNodeId( p );

            if( nodeId == 0 )
                continue;

            Box_i32 absCoords = tree.getAbsDataCoordinates( p );
            absCoords.s -= Vec3_i32( borderDim );
            absCoords.e += Vec3_i32( borderDim );

            if( absCoords.s.z != curMaxZ )
                continue;

            // count how many blocks were already processed
            if( (++counter) % blocksPercentage == 0 )
            {
                int64_t percents = int64_t(counter*100/validBlocks);

                if( percents != oldPercents )
                {
                    LOG_INFO << std::endl;
                    LOG_INFO << "copying original blocks (" << counter << "/" << validBlocks << "): "
                             << percents << "%" << std::flush;
                    oldPercents = percents;
                }else
                    LOG_INFO << "." << std::flush;
            }

            if( absCoords.getDim() != dstBlockAndBordersDim )
                FAIL( "size to read doesn't match size to write" );

            _hddIn->read( absCoords, &src[0] );
            if( _inType == RAW_DER )
                 DataManipulation::removeDerivatives( &src[0], dstBlockAndBordersDim );
            _hddOut->write( nodeId, &src[0] );
        }
    }
    LOG_INFO << std::endl;

// 2. go through all other blocks and aggregate them

    const byte      dstBytesNum      = _dataOut->getBytesNum();
    const Vec3_ui32 dstBlockDimSmall = dstBlockDim / 2;
    const size_t    dstBSSmall = dstBlockDimSmall.getAreaSize()*dstBytesNum;

    src.resize( dstBS );
    std::vector<byte> tmp( dstBS );
    byte* tmpL = &tmp[0];

    byte* dstL = &src[0]; // reuse allocated already memory
    std::vector<byte> dstS8( dstBSSmall*8 );
    byte* dstS[8];
    for( int i = 0; i < 8; ++i )
        dstS[ i ] = &dstS8[ i*dstBSSmall ];

    uint32_t reduceCount = 0;
    while( blocks >= 8 )
    {
        // switch to previous level and process all parents
        pos = tree.getParent( pos );
        blocks /= 8;
        LOG_INFO  << "-----------------------" << std::endl;
        LOG_INFO_ << "parent pos: " << pos << "; "
                  << "parents: " << blocks << std::endl;


        // count actual blocks
        validBlocks = 0;
        for( int64_t p = pos; p < pos+blocks; ++p )
        {
            const uint32_t nodeId = tree.getNodeId( p );
            if( nodeId != 0 )
                validBlocks++;
        }

        counter = 0;
        oldPercents = -1;
        blocksPercentage = hlpFuncs::myClip( validBlocks/100u, 100u, 1000u );
        for( int64_t p = pos+blocks-1; p >= pos; --p )
        {
            const uint32_t parentId = tree.getNodeId( p );

            if( parentId == 0 )
                continue;

            // count how many blocks were already processed
            if( (++counter) % blocksPercentage == 0 )
            {
                int64_t percents = int64_t(counter*100/validBlocks);

                if( percents != oldPercents )
                {
                    LOG_INFO << std::endl;
                    LOG_INFO << "data reduction (" << counter << "/" << validBlocks << "): "
                             << percents << "%" << std::flush;
                    oldPercents = percents;
                }else
                    LOG_INFO << "." << std::flush;
            }

            memset( &dstS8[0], 0, dstS8.size()*sizeof(dstS8[0]) );
            const uint32_t child = tree.getChild( p );
            for( uint32_t i = 0; i < 8; ++i )
            {
                const uint32_t childId = tree.getNodeId( child+i );

                if( childId == 0 )
                    continue;

                _hddOut->read( childId, dstL );
                DataManipulation::stripBorders( dstL, dstBlockDim, borderDim, dstBytesNum );
                DataManipulation::reduceTwice( dstS[i], dstL, dstBlockDim, dstBytesNum );
                reduceCount++;
            }
            memset( dstL, 0, dstBS );
            memset( tmpL, 0, dstBS );
            DataManipulation::combineEight( tmpL, &dstS8[0], dstBlockDimSmall, dstBytesNum );
            DataManipulation::addBorders( dstL, tmpL, dstBlockDim, borderDim, dstBytesNum );
            _hddOut->write( parentId, dstL );
        }
        LOG_INFO << std::endl;
    }
    LOG_INFO << "=======================" << std::endl
             << "Adding borders" << std::endl;

    std::vector<byte> cache;
    uint32_t first = 1;
    uint32_t last  = 9;
    uint32_t dim   = dstBlockDim.x+borderDim*2;
    uint32_t level = 1;
    uint32_t bordersCopied = 0;
    uint32_t bordersAdded  = 0;
    while( borderDim > 0 && last < tree.getSize() ) // for each level of the tree (except leafs) add borders separately
    {
        LOG_INFO << "Level: " << level++ << std::endl;
        dim = (dim - borderDim*2)*2 + borderDim*2;
        const uint32_t dimXY = dim*dim;

        // two layers of blocks along z axiss
        cache.resize( dimXY*(dstBlockDim.z+borderDim)*2*dstBytesNum );
        memset( &cache[0], 0, cache.size()*sizeof(cache[0]) );

        const Vec3_i64  blockStride = dstBlockAndBordersDim;
        const Vec3_i64  cacheStride( dim, dim, (dstBlockDim.z+borderDim)*2 );
        const uint32_t  blockOffset = (dstBlockAndBordersDim.x*(dstBlockAndBordersDim.y + 1) + 1)*borderDim*dstBytesNum;
        const uint32_t  dstOffset =  cache.size()/2 + ((dim + 1)*borderDim)*dstBytesNum; // 1'st stage

        const uint32_t lastZ = dim-borderDim*2;
        for( uint32_t z = 0; z <= lastZ; z += dstBlockDim.z ) // for each z layer (+1 z for the last one)
        {
            if( z != 0 ) // safe copy previous layer down
            {
                const byte* s  = &cache[ cache.size()/2 - dimXY*borderDim*dstBytesNum ];
                      byte* d  = &cache[0];
                const uint32_t size = dimXY*dstBytesNum;
                for( uint32_t i = 0; i < dstBlockDim.z+borderDim; ++i )
                {
                    memcpy( d, s, size );
                    s += size;
                    d += size;
                }
                memset( &cache[cache.size()/2], 0, cache.size()/2*sizeof(cache[0]) );
            }

            // 1st stage: fill another cache layer
            if( z != lastZ )
                for( uint32_t i = first; i < last; ++i )
                {
                    const uint32_t id = tree.getNodeId( i );
                    if( id == 0 )
                        continue;

                    Box_i32 bCoords = tree.getRelativeCoordinates( i );
                    if( bCoords.s.z != static_cast<int32_t>(z) )
                        continue;

                    _hddOut->read( id, dstL );

                    const int64_t dstLocal = (bCoords.s.y*dim + bCoords.s.x)*dstBytesNum;

                    DataManipulation::copyData( &cache[dstOffset + dstLocal], dstL+blockOffset, dstBlockDim, cacheStride, blockStride, dstBytesNum );
                    ++bordersCopied;
                }

            // 2nd stage: copy previous layer with borders back
            if( z != 0 )
                for( uint32_t i = first; i < last; ++i )
                {
                    const uint32_t id = tree.getNodeId( i );
                    if( id == 0 )
                        continue;

                    Box_i32 bCoords = tree.getRelativeCoordinates( i );
                    if( bCoords.s.z != static_cast<int32_t>(z-dstBlockDim.z) )
                        continue;

                    const int32_t srcLocal = (bCoords.s.y*dim + bCoords.s.x)*dstBytesNum;

                    assert( bCoords.getAreaSize() == dstBlockDim.getAreaSize( ));
                    DataManipulation::copyData( dstL, &cache[srcLocal], dstBlockAndBordersDim, blockStride, cacheStride, dstBytesNum );
                    _hddOut->write( id, dstL );
                    ++bordersAdded;
                }
        }

        // next level
        first = tree.getChild( first );
        last  = tree.getChild( last  );
    }
    if( bordersCopied != bordersAdded )
        LOG_INFO << "ERROR: number of blocks copied != number of blocks extended" << std::endl;

    LOG_INFO << "=======================" << std::endl
             << "data reduction was called " << reduceCount
             << " times; block size: " << (int)dstBS << std::endl
             << "blocks with borders copied/extended " << bordersCopied << "/" << bordersAdded << std::endl;

    LOG_INFO_ << "------------- Done converting -------------" << std::endl;
}

} //namespace massVolVis
