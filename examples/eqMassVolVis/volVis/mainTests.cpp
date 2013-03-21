
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "asyncFetcher/gpuCacheIndex.h"

#include <stdlib.h>

#include <lunchbox/debug.h>       // EQASSERT macro

using namespace massVolVis;

int main( const int argc, char** argv )
{
// ----------- cache index tests -----------
    {
    //GPUCacheIndex gi( const uint32_t gpuMemorySize, const uint32_t blockSize, const uint32_t maxCacheDimention );
    GPUCacheIndex gi1( 20*20*20 *1024*256     , 64, 30*64 );
    LBASSERT( gi1.capacity() == 20 * 20 * 20 );
    LBASSERT( gi1.totalCacheTextureDimBlocks() == Vec3_ui32( 20 ));
    LBASSERT( gi1.getBlockCoordinates( 0  ) == Box_i32( Vec3_i32(0), Vec3_i32(64)));
    LBASSERT( gi1.getBlockCoordinates( 20 ).s == Vec3_i32(0,64,0));
    }

    GPUCacheIndex gi2( 500      *1024*1024 / 4, 64, 9*64 );
    LBASSERT( gi2.capacity() == 6 * 9 * 9 ); //486
    LBASSERT( gi2.totalCacheTextureDimBlocks() == Vec3_ui32(6, 9, 9));
    LBASSERT( gi2.getBlockCoordinates( 6*9*3+4 ).s == Vec3_i32(4,0,3)*64);
    LBASSERT( gi2.getBlockCoordinates( 6*9*9-1 ).s == Vec3_i32(5,8,8)*64);

    GPUCacheIndex gi3( 101      *1024*1024/ 4, 64, 64*101 );
    LBASSERT( gi3.capacity() == 1 * 1 * 101 ); // 101
    LBASSERT( gi3.totalCacheTextureDimBlocks() == Vec3_ui32(1, 1, 101));
    LBASSERT( gi3.getBlockCoordinates( 1*1*100 ).s == Vec3_i32(0,0,100)*64);

    GPUCacheIndex gi4( 11*12*13 *1024*1024/ 4, 64, 64*40 );
    LBASSERT( gi4.capacity() == 2 * 22 * 39 ); // 1716
    LBASSERT( gi4.totalCacheTextureDimBlocks() == Vec3_ui32(2, 22, 39));
    LBASSERT( gi4.getBlockCoordinates( 2*22*10+2*11+1 ).s == Vec3_i32(1,11,10)*64);


    LBWARN << "All tests are done" << std::endl;
    return 0;
}







