
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__GPU_CACHE_VALUE_H
#define MASS_VOL__GPU_CACHE_VALUE_H

#include <msv/tree/nodeId.h>

#include <stdint.h>
#include <vector>

/**
 * Describes one element in the GPU cache.
 *  - is element in use?
 *  - when was last time accessed;
 *  - what is data position in the 1D chache array.
 */
struct GPUCacheValue
{
    uint64_t iteration; // smaller means older data
    NodeId   nodeId;    // data id

    explicit GPUCacheValue( uint32_t pos = 0 )
        : iteration( 0 ), nodeId( 0 ), _pos( pos ) {}

    bool isFree() const { return iteration == 0; }

    void set( const uint64_t iteration_, const NodeId nodeId_ )
    {
        iteration = iteration_;
        nodeId    = nodeId_;
    }

    void reset() { iteration = 0; nodeId = 0; }

    uint32_t getPos() const { return _pos; }

private:
    uint32_t _pos;
};


typedef std::vector<GPUCacheValue> GPUCacheValueVec;

#endif // MASS_VOL__GPU_CACHE_VALUE_H
    