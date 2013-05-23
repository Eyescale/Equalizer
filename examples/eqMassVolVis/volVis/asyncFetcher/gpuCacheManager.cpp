
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#include "gpuCacheManager.h"

#include "gpuAsyncLoader.h"

#include "gpuCacheIndex.h"
#include <msv/tree/volumeTreeBase.h>
#include <numeric> // std::accumulate

namespace massVolVis
{

GPUCacheManager::GPUCacheManager( constGPUCacheIndexSPtr cacheIndex, RAMPoolSPtr ramPool, const byte bytesNum,
                                   Window* wnd )
    :_iteration( 1 )
    ,_cacheIndex( cacheIndex )
    ,_gpuLoader(  new GPUAsyncLoader( wnd ))
    ,_nodeIdBeingLoaded( 0 )
    ,_cachePosBeingLoaded( 0 )
{
    LBASSERT( _cacheIndex );

    _gpuLoader->start();

    const GPULoadRespond respond = _gpuLoader->readLoadRespond();
    if( respond.status.value != GPULoadStatus::INITIALIZED )
        LBERROR << "Incorrect respond from GPU Async Loader" << std::endl;

    _gpuLoader->postCommand( GPUCommand::PAUSE );
    _gpuLoader->initialize( ramPool, cacheIndex, bytesNum );
    _resize();
    LBWARN << " new GPUCacheManager, updating GPU Cache" << std::endl;
    _gpuLoader->postCommand( GPUCommand::UPDATE );
    _gpuLoader->postCommand( GPUCommand::RESUME );
}


void GPUCacheManager::setDataVersion( const uint32_t version )
{
    _gpuLoader->setDataVersion( version );
}


GLuint GPUCacheManager::getStorageTextureId() const
{
    return _gpuLoader->getStorageTextureId();
}


void GPUCacheManager::_resize()
{
    _cacheValues.clear();
    _cacheValues.reserve( _cacheIndex->capacity() );
    for( size_t i = 0; i < _cacheIndex->capacity(); ++i )
        _cacheValues.push_back( GPUCacheValue( i ));

    _cacheValuesTmp.resize( _cacheValues.size() );
}

void GPUCacheManager::_processRespond( const GPULoadRespond& respond )
{
    switch( respond.status.value )
    {
        case GPULoadStatus::STARTED:
//                LBWARN << "GPUCacheManager: loading started: " << respond.nodeId << std::endl;
            LBASSERT( respond.posOnGPU < _cacheValues.size( ));
            _nodeIdBeingLoaded   = respond.nodeId;
            _cachePosBeingLoaded = respond.posOnGPU;
            _cacheValues[ respond.posOnGPU ].set( _iteration, respond.nodeId );
            break;

        case GPULoadStatus::FINISHED:
//                LBWARN << "GPUCacheManager: loading confirmed: " << respond.nodeId << " / " << respond.posOnGPU << std::endl;
            LBASSERT( respond.posOnGPU < _cacheValues.size( ));
            _cacheValues[  respond.posOnGPU  ].set( _iteration, respond.nodeId );
            _usedElements[ respond.nodeId ] = GpuLocation( respond.posOnGPU );
            _nodeIdBeingLoaded = 0;
            break;

        case GPULoadStatus::FAILED:
//                LBWARN << "GPUCacheManager: loading failed: " << respond.nodeId << std::endl;
            LBASSERT( respond.posOnGPU < _cacheValues.size( ));
            _cacheValues[ respond.posOnGPU ].reset();
            _nodeIdBeingLoaded = 0;
            break;

        default:
            LBERROR << "Incorrect command from GPU loader" << std::endl;
    }
}


void GPUCacheManager::_processGPULoaderResponces()
{
    //update information about used by GPU memory

    GPULoadRespond respond;

    while( _gpuLoader->tryReadLoadRespond( respond ))
    {
        _processRespond( respond );
    }
}


void GPUCacheManager::_processGPULoaderResponcesAndPause()
{
    _gpuLoader->postCommand( GPUCommand::PAUSE_AND_REPORT );
    _gpuLoader->clearLoadRequests();

    while( true )
    {
        GPULoadRespond respond = _gpuLoader->readLoadRespond();
        if( respond.status.value == GPULoadStatus::PAUSED )
            return;
        _processRespond( respond );
    }
}


/**
 *  Sorting comparison for LRU chache values kick out.
 */
int GPUCacheValueIterationCmp( const void * aP, const void * bP )
{
    const GPUCacheValue* a = static_cast< const GPUCacheValue* >( aP );
    const GPUCacheValue* b = static_cast< const GPUCacheValue* >( bP );

    if( a->iteration == b->iteration )
        return 0;

    return ( a->iteration < b->iteration ) ? -1 : 1;
}


void GPUCacheManager::updateFront( const NodeIdPosVec& desiredIds )
{
    _gpuLoader->postCommand( GPUCommand::PAUSE );
    _gpuLoader->clearLoadRequests();

    _processGPULoaderResponces();

    _iteration++;

    // update GPU iterations of currently used data and find not loaded Ids
    _newIds.clear();
    _newIds.reserve( desiredIds.size() );

    // protect GPU slot that is being used for async loading
    if( _nodeIdBeingLoaded != 0 )
        _cacheValues[ _cachePosBeingLoaded ].iteration = _iteration;

    _requests.clear();
    for( size_t i = 0; i < desiredIds.size(); ++i )
    {
        const NodeIdPos& testNodeId = desiredIds[i];
        NodeIdHash::const_iterator nodeIdIterator = _usedElements.find( testNodeId.id );
        if( nodeIdIterator != _usedElements.end() )
        {
            const GpuLocation& nodeId = nodeIdIterator->second;
            const uint32_t cachePos = nodeId.posOnGpu;
            LBASSERT( cachePos < _cacheValues.size() );
            LBASSERT( _cacheValues[ cachePos ].nodeId == testNodeId.id );

            _cacheValues[ cachePos ].iteration = _iteration;
        }else
        {
            _newIds.push_back( desiredIds[i] );
        }
    }

    // allocate space on GPU (find what will not be used and can be pushed away)
    _releaseIds.clear();
    _releaseIds.reserve( _newIds.size() );

    LBASSERT( _cacheValuesTmp.size() == _cacheValues.size() );
    memcpy( &_cacheValuesTmp[0], &_cacheValues[0], sizeof(GPUCacheValue) * _cacheValues.size());
    qsort( &_cacheValuesTmp[0], _cacheValuesTmp.size(), sizeof(GPUCacheValue), GPUCacheValueIterationCmp );

    uint32_t releaseCount = 0;
    while( _cacheValuesTmp[ releaseCount ].iteration < _iteration && // remove only older data
           releaseCount < _cacheValuesTmp.size() &&                  // remove less than cache size
           releaseCount < _newIds.size())                            // remove not more than necessary
    {
        if( !_cacheValuesTmp[ releaseCount ].isFree() )
        {
            _releaseIds.push_back( _cacheValuesTmp[ releaseCount ].nodeId );
        }
        releaseCount++;
    }

    // remove unused Ids
    for( size_t i = 0; i < _releaseIds.size(); ++i )
    {
        NodeIdHash::iterator nodeIdIterator = _usedElements.find( _releaseIds[i] );
        if( nodeIdIterator == _usedElements.end() )
        {
            LBERROR << "element is not in the cache - not possible to release it" << std::endl;
        }else
        {
            const uint32_t cachePos = nodeIdIterator->second.posOnGpu;
            LBASSERT( cachePos < _cacheValues.size( ));
            LBASSERT( _cacheValues[ cachePos ].nodeId == _releaseIds[i] );

            _cacheValues[ cachePos ].reset();
            _usedElements.erase( nodeIdIterator );
        }
    }

    // unlock data in RAM
    if( _releaseIds.size() > 0 )
        _gpuLoader->releaseRAMData( _releaseIds );

    // schedule new loadings
    _requests.reserve( _requests.size() + releaseCount );

    LBASSERT( releaseCount <= _cacheValues.size() );
    LBASSERT( releaseCount <= _newIds.size() );

    for( size_t i = 0; i < releaseCount; ++i )
    {
        if( _newIds[i].id == _nodeIdBeingLoaded )
            continue;

        const uint32_t cachePos = _cacheValuesTmp[i].getPos();
        LBASSERT( _cacheValues[ cachePos ].isFree() );
        _requests.push_back(
            GPULoadRequest( _newIds[i].id, cachePos, _newIds[i].treePos, false ));
    }

    _gpuLoader->clearRAMLoadRequests();

    if( _requests.size() > 0 )
        _gpuLoader->postLoadRequestVec( _requests );

    _gpuLoader->postCommand( GPUCommand::RESUME );
}


bool GPUCacheManager::hasNodeOnGPU( const NodeId nodeId ) const
{
//    _processGPULoaderResponces(); // don't do this every time?

    NodeIdHash::const_iterator nodeIdIterator = _usedElements.find( nodeId );

    if( nodeIdIterator == _usedElements.end( ))
        return false;

    return true;
}


Box_i32 GPUCacheManager::nodeParametersOnGPU( const NodeId nodeId ) const
{
    NodeIdHash::const_iterator nodeIdIterator = _usedElements.find( nodeId );
    if( nodeIdIterator == _usedElements.end( ))
        return Box_i32();

    const uint32_t cachePos = nodeIdIterator->second.posOnGpu;
    LBASSERT( cachePos < _cacheValues.size( ));
    LBASSERT( _cacheValues[ cachePos ].nodeId == nodeId );

    return _cacheIndex->getBlockCoordinates( cachePos );
}


Vec3_f GPUCacheManager::getGPUMemoryOffset( const NodeId nodeId ) const
{
    NodeIdHash::const_iterator nodeIdIterator = _usedElements.find( nodeId );
    if( nodeIdIterator == _usedElements.end( ))
        return Vec3_f();

    const uint32_t cachePos = nodeIdIterator->second.posOnGpu;
    LBASSERT( cachePos < _cacheValues.size( ));
    LBASSERT( _cacheValues[ cachePos ].nodeId == nodeId );

    return _cacheIndex->getGPUMemoryOffset( cachePos );
}


const Vec3_ui32& GPUCacheManager::totalCacheTextureDim() const
{
    return _cacheIndex->totalCacheTextureDim();
}


const Vec3_f& GPUCacheManager::getBlockScale() const
{
    return _cacheIndex->getBlockScale();
}


const Vec3_f& GPUCacheManager::getVoxelScale() const
{
    return _cacheIndex->getVoxelScale();
}


uint32_t GPUCacheManager::capacity() const
{
    return _cacheIndex->capacity();
}


void GPUCacheManager::resetGPUCache()
{
    LBWARN << " Reseting GPU Cache" << std::endl;
    // pause and clear GPU Loader
    _gpuLoader->postCommand( GPUCommand::PAUSE );
    _gpuLoader->clearLoadRequests();

    // clean GPU Loader responds
    _processGPULoaderResponces();

    // clean cache data
    _usedElements.clear();
    _resize();

    // reset LRU timer
    _iteration = 1;

    // clear requests to RAM
    _gpuLoader->clearRAMLoadRequests();

    // update texture size if necessary
    _gpuLoader->postCommand( GPUCommand::UPDATE );

    // resume GPU loader
    _gpuLoader->postCommand( GPUCommand::RESUME );
}


}
