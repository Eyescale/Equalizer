
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "gpuCacheIndex.h"

#include <lunchbox/debug.h>       // EQASSERT macro

#include <math.h> // cbrt

namespace massVolVis
{

namespace
{
Vec3_ui32 _findBest3DTextureMatch( const uint32_t maxBlocks, const uint32_t maxDim )
{
    // check if we have more memory than it is allowed to allocate due to 3D texture size limitations
    if( maxDim * maxDim * maxDim <= maxBlocks )
        return Vec3_ui32( maxDim );

    // else: check if all the memory would fit to 1 dimentional array
    if( maxBlocks <= maxDim )
        return Vec3_ui32( 1, 1, maxDim );

    // else: check if cube texture would do
    const uint32_t s = static_cast<uint32_t>( cbrt( maxBlocks ));
    LBASSERT( (s+1)*(s+1)*(s+1) > maxBlocks );

    Vec3_ui32 textureSize = Vec3_ui32( s );
    uint32_t maxArea = s * s * s;

    LBASSERT( maxArea <= maxBlocks );

    if( maxArea == maxBlocks )
        return textureSize;

    // else: find the best parallelepipid that can fit maxBlocks with the least memmory waste
    for( uint32_t z = maxDim; z >= 1; --z )
    {
        if( z * maxDim * maxDim <= maxArea ) // z is too small already
            break;

        const uint32_t maxY = std::min( maxBlocks / z, maxDim ); // [y*z <= maxBlocks] => [x >= 1]

        for( uint32_t y = maxY; y >= 1; --y )
        {
            const uint32_t yz = y * z;
            const uint32_t x  = std::min( maxBlocks / yz, maxDim );
            const uint32_t area = x * yz;

            if( area > maxArea )
            {
                maxArea = area;
                textureSize = Vec3_ui32( x, y, z );

                if( area == maxBlocks )
                    return textureSize;
            }

            if( x == maxDim ) // y is too small already, x (and area) will not increase anymore
                break;
        }
    }
    return textureSize;
}


Vec3_i32 _getBlockStart( const uint32_t id, const Vec3_ui32& cacheTextureDimBlocks, const uint32_t blockDim )
{
    const uint32_t dx  = cacheTextureDimBlocks.x;
    const uint32_t dxy = cacheTextureDimBlocks.x * cacheTextureDimBlocks.y;

    const uint32_t z =  id / dxy;
    const uint32_t y = (id - z*dxy ) / dx;
    const uint32_t x =  id - z*dxy - y*dx;

    return Vec3_i32( x, y, z ) * blockDim;
}
}// namespace


GPUCacheIndex::GPUCacheIndex( const uint32_t gpuMemorySize, const uint32_t blockDim, const uint32_t maxCacheDim )
    : _capacity( 0 )
    , _blockDim( blockDim )
{
    const uint32_t maxBlocks = gpuMemorySize / (blockDim * blockDim * blockDim);
    const uint32_t maxDim    = maxCacheDim /  blockDim;

    _cacheTextureDimBlocks = _findBest3DTextureMatch( maxBlocks, maxDim );

    LBASSERT( _cacheTextureDimBlocks.x <= maxDim &&
              _cacheTextureDimBlocks.y <= maxDim &&
              _cacheTextureDimBlocks.z <= maxDim    );

    _capacity = _cacheTextureDimBlocks.x * _cacheTextureDimBlocks.y * _cacheTextureDimBlocks.z;

    LBASSERT( _capacity <= maxBlocks );

    _cacheTextureDim = _cacheTextureDimBlocks * blockDim;
    _blockScale = Vec3_f( 1.f/_cacheTextureDimBlocks.x, 1.f/_cacheTextureDimBlocks.y, 1.f/_cacheTextureDimBlocks.z );
    _voxelScale = Vec3_f( 1.f/_cacheTextureDim.x,       1.f/_cacheTextureDim.y,       1.f/_cacheTextureDim.z       );
}


bool GPUCacheIndex::_isWithinCapacity( const uint32_t id ) const
{
    if( id < capacity( ))
        return true;

    LBERROR << "There is no such memory block on GPU. id: " << id << " capacity: " << capacity() << std::endl;
    return false;
}


Box_i32 GPUCacheIndex::getBlockCoordinates( const uint32_t id ) const
{
    if( !_isWithinCapacity( id ))
        return Box_i32();

    const Vec3_i32 start = _getBlockStart( id, _cacheTextureDimBlocks, _blockDim );

    return Box_i32( start, start + _blockDim );
}


Vec3_f GPUCacheIndex::getGPUMemoryOffset( const uint32_t id ) const
{
    if( !_isWithinCapacity( id ))
        return Vec3_f();

    const Vec3_i32 start = _getBlockStart( id, _cacheTextureDimBlocks, _blockDim );

    return Vec3_f(  static_cast<float>( start.x ) / _cacheTextureDim.x,
                    static_cast<float>( start.y ) / _cacheTextureDim.y,
                    static_cast<float>( start.z ) / _cacheTextureDim.z  ); 
}


bool GPUCacheIndex::operator == ( const GPUCacheIndex& other ) const
{
    return _cacheTextureDim == other._cacheTextureDim &&
           _blockDim        == other._blockDim;
}


const GPUCacheIndex& GPUCacheIndex::operator = ( const GPUCacheIndex& from )
{
    _cacheTextureDimBlocks = from._cacheTextureDimBlocks;
    _cacheTextureDim       = from._cacheTextureDim;
    _capacity              = from._capacity;
    _blockDim              = from._blockDim;
    return *this;
}


} //namespace massVolVis
