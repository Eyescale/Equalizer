
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__GPU_CACHE_INDEX_H
#define MASS_VOL__GPU_CACHE_INDEX_H

#include <msv/types/box.h>

namespace massVolVis
{

/**
 * Mapping of linear index of a block in the GPU memory to 3D index of actual storage.
 *
 * Data is stored on the GPU in a single 3D texture, which contain multiple blocks 
 * of data.
 */
class GPUCacheIndex
{
public:
    /**
     * Computes optimal 3D texture size for a given memory size and block size.
     * This function has to be called first.
     *
     * \param gpuMemorySize     - has to be in bytes and it has to be pre-divided by number of bytes per data element.
     * \param blockDim          - dimention of a cubical data block (with borders).
     * \param maxCacheDimention - maximum allowed size of 3D texture on GPU.
     */
    GPUCacheIndex( const uint32_t gpuMemorySize, const uint32_t blockDim, const uint32_t maxCacheDim );

    /**
     * Total size of 3D texture cache in the GPU memory in blocks.
     */ 
    const Vec3_ui32& totalCacheTextureDimBlocks() const { return _cacheTextureDimBlocks; }

    /**
     * Total size of 3D texture cache in the GPU memory.
     */ 
    const Vec3_ui32& totalCacheTextureDim() const { return _cacheTextureDim; }

    /**
     * Returns coordinates within cache texture based on a given index.
     *
     * \param id - linear index. Has to be less than "capacity()".
     */
    Box_i32 getBlockCoordinates( const uint32_t id ) const;


    /**
     * Returns relative memory coordinates of a texture block start 
     * within cache texture, based on a given index.
     *
     * \param id - linear index. Has to be less than "capacity()".
     */
    Vec3_f getGPUMemoryOffset( const uint32_t id ) const;


    /**
     * \return Block dimetion on GPU (includes borders)
     */
    uint32_t getBlockDim() const { return _blockDim; }

    /**
     * Maximum number of blocks that is storred in the cache.
     */ 
     uint32_t capacity() const { return _capacity; }

    /**
     * Relative size of a block compared to the total texture size
     */ 
    const Vec3_f&   getBlockScale() const { return _blockScale; }

    /**
     * Relative size of a voxel compared to the total texture size
     */ 
    const Vec3_f&   getVoxelScale() const { return _voxelScale; }


    bool operator == ( const GPUCacheIndex& other ) const;

    const GPUCacheIndex& operator = ( const GPUCacheIndex& from );

private:
    GPUCacheIndex(){}

    /**
     * Checks if a given block id lay within avaliable capacity
     */ 
    bool _isWithinCapacity( const uint32_t id ) const;

    Vec3_ui32   _cacheTextureDim;       //!< dimentions of GPU cache in voxels
    Vec3_ui32   _cacheTextureDimBlocks; //!< dimentions of GPU cache in blocks
    uint32_t    _capacity;              //!< max number of blocks that can be stored on GPU
    uint32_t    _blockDim;              //!< Block dimention on GPU (includes borders)
    Vec3_f      _blockScale;            //!< relative block dimintions if entire texture has size 1.0x1.0x1.0
    Vec3_f      _voxelScale;            //!< relative voxel dimintions if entire texture has size 1.0x1.0x1.0
};

} //namespace massVolVis

#endif //MASS_VOL__GPU_CACHE_INDEX_H

