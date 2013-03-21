
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#include "node.h"

#include "volumeInfo.h"

#include "../asyncFetcher/ramPool.h"

#include <msv/IO/volumeFileInfo.h>
#include <msv/tree/volumeTreeBase.h>
#include <msv/IO/dataHDDIOOctree.h>
#include <msv/IO/dataHDDIO.h>

#include "config.h"
#include "error.h"


namespace massVolVis
{


Node::Node( eq::Config* parent )
    : eq::Node( parent )
    , _volumeInfo( new VolumeInfo() )
    , _fileNameVersion( 0 )
    , _fileNameVersionTested( 0 )
    , _ramPool( new RAMPool() )
{
}


void Node::_frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
// check if file name parameter has changed and setup ramPool
    _volumeInfo->sync();

    const uint32_t newFileNameVersion = _volumeInfo->getModelFileNameVersion();
    if( _fileNameVersionTested == newFileNameVersion )
        return;

// Stop RAM loader
    _ramPool->stopDataLoading();

// Init new file info
    const std::string& fileName = _volumeInfo->getModelFileName();
    _fileNameVersionTested = newFileNameVersion;

    LBWARN << "Opening new file: " << fileName.c_str() << std::endl;

    VolumeFileInfoSPtr volFileInfo( new VolumeFileInfo() );
    if( !volFileInfo->load( fileName ))
    {
        LBERROR << "Can't load file: " << fileName.c_str() << std::endl;
        return;
    }
    if( !volFileInfo->isAttributeSet( VolumeFileInfo::SOURCE_DIMS ) ||
        !volFileInfo->isAttributeSet( VolumeFileInfo::BLOCK_DIM   ) ||
        !volFileInfo->isAttributeSet( VolumeFileInfo::BYTES       ) ||
        !volFileInfo->isAttributeSet( VolumeFileInfo::DATA_FILE   )   )
    {
        LBERROR << "Can't read some data parameters from file: " << fileName.c_str() << std::endl;
        return;
    }
    if( volFileInfo->getBlockDim() < 16 ||
        volFileInfo->getBytesNum() <  1    )
    {
        LBERROR << "Some volume parameters are incorrect" << std::endl;
        return;
    }

    _volFileInfo = volFileInfo;

    LBWARN << "Loading file: " << *_volFileInfo << std::endl;

    DataHDDIOSPtr dataHDDIO = _volFileInfo->createDataHDDIO();
    LBASSERT( dataHDDIO );

    _volTree = dataHDDIO->getTree();
    LBASSERT( _volTree );

    LBWARN << _volTree->getInfoString().c_str() << std::endl;
// Resume RAM loader
    // TODO: set RAM limits correctly 
//    _ramPool->startDataLoading( dataHDDIO, 3*1024, _fileNameVersionTested );
    _ramPool->startDataLoading( dataHDDIO, 1024, _fileNameVersionTested );
    _fileNameVersion = _fileNameVersionTested;

    // set TF from original data description if available
    TransferFunction tf;
    if( dataHDDIO->isTfFileNameDefined( ) &&
        tf.load( dataHDDIO->getTFFileName( )))
    {
        _volumeInfo->setTransferFunction( tf );
    }else
        _volumeInfo->resetTransferFunction();

    _volumeInfo->commit();
}


void Node::frameStart(  const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    _frameStart( frameId, frameNumber );

    eq::Node::frameStart( frameId, frameNumber );
}


bool Node::configInit( const eq::uint128_t& initId )
{
    if( !eq::Node::configInit( initId ))
        return false;

    Config* config = static_cast< Config* >( getConfig( ));
    if( !config->mapData( initId ))
    {
        setError( ERROR_VOLVIS_MAP_CONFIG_OBJECT_FAILED );
        return false;
    }

    if( !config->mapObject( _volumeInfo.get(), config->getInitData().getVolumeInfoId( )))
    {
        setError( ERROR_VOLVIS_MAP_VOLUME_INFO_OBJECT_FAILED );
        return false;
    }

    return true;
}


bool Node::configExit()
{
    Config* config = static_cast< Config* >( getConfig( ));
    config->unmapObject( _volumeInfo.get() );

    return eq::Node::configExit();
}


}//namespace massVolVis
