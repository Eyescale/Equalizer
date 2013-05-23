
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__GPU_CACHE_MANAGER_H
#define MASS_VOL__GPU_CACHE_MANAGER_H

#include "gpuCacheValue.h"

#include "gpuCommands.h"

#include <msv/tree/nodeId.h>

#include <msv/types/box.h>

#include <eq/client/gl.h>

#include <lunchbox/hash.h>

#include <boost/shared_ptr.hpp>

namespace massVolVis
{

class Window;
class RAMPool;
class GPUCacheIndex;
class GPUAsyncLoader;

typedef boost::shared_ptr< const GPUCacheIndex > constGPUCacheIndexSPtr;

typedef boost::shared_ptr< GPUAsyncLoader > GPUAsyncLoaderSPtr;

typedef boost::shared_ptr< RAMPool > RAMPoolSPtr;


struct GpuLocation
{
    GpuLocation() : posOnGpu( 0 ) {}

    explicit GpuLocation( uint32_t posOnGpu_ )
        : posOnGpu( posOnGpu_ ){}

    uint32_t posOnGpu;
};

/**
 * Estimates what has to be loaded/unloaded to/from GPU cache.
 * Communicates with gpuAsyncLoader.
 */
class GPUCacheManager
{
public:
    typedef std::vector< bool > LevelsResetVec;

    GPUCacheManager( constGPUCacheIndexSPtr cacheIndex, RAMPoolSPtr ramPool, const byte bytesNum,
                     Window* wnd );

    /**
     * - recieves new front from the model (model has to maintain 2 fronts - desired and rendering)
     * - asks gpuAsyncFetcher for new textures
     * - tells ramPool to release blocks that were pushed from GPU (through gpuAsyncFetcher call)
     */
    void updateFront( const NodeIdPosVec& desiredIds );

    /**
     * Returns true if nodeId is on GPU
     */
    bool hasNodeOnGPU( const NodeId nodeId ) const;

    /**
     * Returns cube location of a node on GPU
     */
    Box_i32 nodeParametersOnGPU( const NodeId nodeId ) const;

    /**
     * Returns relative cube start location of a node on GPU
     */
    Vec3_f  getGPUMemoryOffset(  const NodeId nodeId ) const;

    const Vec3_ui32& totalCacheTextureDim() const;

    const Vec3_f&   getBlockScale() const;

    const Vec3_f&   getVoxelScale() const;

    uint32_t capacity() const;

    /**
     * When new data set is used, this function should be called 
     * to reset GPU Memory content.
     */
    void resetGPUCache();

    void setDataVersion( const uint32_t version );

    GLuint getStorageTextureId() const;

private:
    void _resize();
    void _processRespond( const GPULoadRespond& respond );
    void _processGPULoaderResponces();
    void _processGPULoaderResponcesAndPause();

    uint64_t _iteration;    //!< local time stamp for LRU

    typedef stde::hash_map< NodeId, GpuLocation > NodeIdHash;

    NodeIdHash          _usedElements;   //!< node Ids that are loaded to GPU (poit to location in _cacheValues)
    GPUCacheValueVec    _cacheValues;    //!< linearly mapped GPU blocks 

    constGPUCacheIndexSPtr _cacheIndex;
    GPUAsyncLoaderSPtr     _gpuLoader;

    NodeId            _nodeIdBeingLoaded;   //!< used to prevent multiple loading of the same block
    uint32_t          _cachePosBeingLoaded; //!< used to prevent multiple loading of the same block

// tmp per-updateFront function call data
    NodeIdPosVec      _newIds;          //!< requested by updateFront Ids
    NodeIdVec         _releaseIds;      //!< released according LRU Ids
    GPULoadRequestVec _requests;        //!< requests to GPU loader; used to reduce number of GPU loader locks
    GPUCacheValueVec  _cacheValuesTmp;  //!< tmp copy of _cacheValues; should have the same size
};

typedef boost::shared_ptr< GPUCacheManager > GPUCacheManagerSPtr;

} //namespace massVolVis

#endif //MASS_VOL__GPU_CACHE_MANAGER_H

