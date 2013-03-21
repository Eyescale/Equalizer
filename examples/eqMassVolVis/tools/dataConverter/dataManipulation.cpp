
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataManipulation.h"

#include <msv/util/debug.h>

#include <string.h> //memcpy

namespace massVolVis
{

template< typename T >
void _reduceTwice(  T* dst, const T* src, const Vec3_ui32& srcDim );

template< typename T >
void _combineEight( T* dst, const T* src, const Vec3_ui32& srcDim );

template< typename T >
void _stripBorders( T* data, const Vec3_ui32& dim, byte bDim );

template< typename T >
void _addBorders( T* dst, const T* src, const Vec3_ui32& dim, byte bDim );


namespace
{
/**
 *  Checks that dimensions are suitable for reduction in two times.
 */
void _checkDimensions( const Vec3_ui32& srcDim )
{
    if(!( srcDim > Vec3_ui32( 0, 0, 0 ))) FAIL( "Dimensions of the block have to be non zero" );

    if( (srcDim / 2 ) * 2 != srcDim )   FAIL( "Dimensions of the block have to be multiples of 2" );
}
}// namespace

void DataManipulation::reduceTwice( void* dst, const void* src, const Vec3_ui32& srcDim, const byte bytes )
{
    _checkDimensions( srcDim );

    switch( bytes )
    {
        case 1:
            _reduceTwice( reinterpret_cast<          byte*>(dst),
                          reinterpret_cast<const     byte*>(src), srcDim );
            break;
        case 2:
            _reduceTwice( reinterpret_cast<      uint16_t*>(dst),
                          reinterpret_cast<const uint16_t*>(src), srcDim );
            break;
        default:
            FAIL( "There can be only 1 or 2 bytes per data value" );
    }
}


void DataManipulation::combineEight( void* dst, const void* src, const Vec3_ui32& srcDim, const byte bytes )
{
    _checkDimensions( srcDim );

    switch( bytes )
    {
        case 1:
            _combineEight( reinterpret_cast<          byte*>(dst),
                           reinterpret_cast<const     byte*>(src), srcDim );
            break;
        case 2:
            _combineEight( reinterpret_cast<      uint16_t*>(dst),
                           reinterpret_cast<const uint16_t*>(src), srcDim );
            break;
        default:
            FAIL( "There can be only 1 or 2 bytes per data value" );
    }
}


void DataManipulation::stripBorders( void* data, const Vec3_ui32& blockDim, byte borderDim, byte bytesNum )
{
    if( borderDim == 0 )
        return;

    _checkDimensions( blockDim );

    switch( bytesNum )
    {
        case 1:
            _stripBorders( reinterpret_cast<    byte*>(data), blockDim, borderDim );
            break;
        case 2:
            _stripBorders( reinterpret_cast<uint16_t*>(data), blockDim, borderDim );
            break;
        default:
            FAIL( "There can be only 1 or 2 bytes per data value" );
    }
}


void DataManipulation::addBorders( void* dst, const void* src, const Vec3_ui32& blockDim, byte borderDim, byte bytesNum )
{
    if( borderDim == 0 )
        return;

    _checkDimensions( blockDim );

    switch( bytesNum )
    {
        case 1:
            _addBorders( reinterpret_cast<    byte*>(dst), reinterpret_cast<const     byte*>(src), blockDim, borderDim );
            break;
        case 2:
            _addBorders( reinterpret_cast<uint16_t*>(dst), reinterpret_cast<const uint16_t*>(src), blockDim, borderDim );
            break;
        default:
            FAIL( "There can be only 1 or 2 bytes per data value" );
    }
}


template< typename T >
void _copyData( T* dst, const T* src, const Vec3_ui32& srcDim, const Vec3_i64& dstStride, const Vec3_i64& srcStride )
{
    for( size_t z = 0; z < srcDim.z; ++z )
    {
              T* dstLine = dst + dstStride.x*dstStride.y*z;
        const T* srcLine = src + srcStride.x*srcStride.y*z;

        for( size_t y = 0; y < srcDim.y; ++y )
        {
            memcpy( dstLine, srcLine, srcDim.x*sizeof(T) );

            dstLine += dstStride.x;
            srcLine += srcStride.x;
        }
    }
}

void DataManipulation::copyData( void* dst, const void* src, const Vec3_ui32& srcDim,
                const Vec3_i64&  dstStride, const Vec3_i64& srcStride, byte bytesNum )
{
    switch( bytesNum )
    {
        case 1:
            _copyData( reinterpret_cast<    byte*>(dst), reinterpret_cast<const     byte*>(src), srcDim, dstStride, srcStride );
            break;
        case 2:
            _copyData( reinterpret_cast<uint16_t*>(dst), reinterpret_cast<const uint16_t*>(src), srcDim, dstStride, srcStride );
            break;
        default:
            FAIL( "There can be only 1 or 2 bytes per data value" );
    }
}


template< typename T >
void _stripBorders( T* data, const Vec3_ui32& dim, byte bDim )
{
    Vec3_i64 srcStride = dim + bDim*2;
    Vec3_i64 dstStride = dim;
    int64_t srcShift = (srcStride.x*srcStride.y + srcStride.x + 1)*bDim;

    _copyData( data, data + srcShift, dim, dstStride, srcStride );
}


template< typename T >
void _addBorders( T* dst, const T* src, const Vec3_ui32& dim, byte bDim )
{
    Vec3_i64 srcStride = dim;
    Vec3_i64 dstStride = dim + bDim*2;
    int64_t srcShift = 0;
    int64_t dstShift = (dstStride.x*dstStride.y + dstStride.x + 1)*bDim;

    _copyData( dst + dstShift, src + srcShift, dim, dstStride, srcStride );
}


template< typename T >
void _reduceTwice( T* dst, const T* src, const Vec3_ui32& srcDim )
{
    const Vec3_ui32 dstDim = srcDim / 2;

    for( size_t z = 0; z < dstDim.z; ++z )
    {
              T* dstSlice  = dst       + dstDim.w*dstDim.h * z;

        const T* srcSlice1 = src       + srcDim.w*srcDim.h * z*2; // source is twice larger
        const T* srcSlice2 = srcSlice1 + srcDim.w*srcDim.h;     // next source line

        for( size_t y = 0; y < dstDim.y; ++y )
        {
                  T* dstLine  = dstSlice  + dstDim.w * y;

            const T* srcLine1 = srcSlice1 + srcDim.w * y*2;
            const T* srcLine2 = srcSlice2 + srcDim.w * y*2;

            const T* srcLine3 = srcLine1  + srcDim.w;
            const T* srcLine4 = srcLine2  + srcDim.w;

            for( size_t x = 0; x < dstDim.x; ++x )
            {
                dstLine[x] = T(( uint32_t(srcLine1[x*2]) + srcLine1[x*2+1] +
                                          srcLine3[x*2]  + srcLine3[x*2+1] +
                                          srcLine2[x*2]  + srcLine2[x*2+1] +
                                          srcLine4[x*2]  + srcLine4[x*2+1]  ) / 8 );
            }
        }
    }
}


template< typename T >
void _copyBlock( T* dst, const T* src, const Vec3_ui32& dstDim, const Vec3_ui32& srcDim )
{
    for( size_t z = 0; z < srcDim.z; ++z )
    {
              T* dstLine = dst + dstDim.x*dstDim.y*z;
        const T* srcLine = src + srcDim.x*srcDim.y*z;

        for( size_t y = 0; y < srcDim.y; ++y )
        {
            memcpy( dstLine, srcLine, srcDim.x*sizeof(T) );

            dstLine += dstDim.y;
            srcLine += srcDim.y;
        }
    }
}


template< typename T >
void _combineEight( T* dst, const T* srcPlain, const Vec3_ui32& srcDim )
{
    const Vec3_ui32 dstDim = srcDim * 2;

    T* dst_[8];
    dst_[0] = dst;
    dst_[1] = dst_[0] + srcDim.x;
    dst_[2] = dst     + dstDim.x*srcDim.y;
    dst_[3] = dst_[2] + srcDim.x;

    const int lowerPartSize = dstDim.x*dstDim.y*srcDim.z;

    dst_[4] = dst_[0] + lowerPartSize;
    dst_[5] = dst_[1] + lowerPartSize;
    dst_[6] = dst_[2] + lowerPartSize;
    dst_[7] = dst_[3] + lowerPartSize;

    const uint64_t srcSize = srcDim.getAreaSize();

    for( int i = 0; i < 8; ++i )
        _copyBlock( dst_[i], srcPlain + srcSize*i, dstDim, srcDim );
}


void DataManipulation::removeDerivatives( byte* src, const Vec3_ui32& srcDim )
{
    const size_t size = srcDim.getAreaSize()*4;

    for( size_t si = 3, di = 0; si < size; si+=4, ++di )
        src[di] = src[si];
}


} //namespace massVolVis
