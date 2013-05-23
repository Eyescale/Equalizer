
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__DATA_MANIPULATION_H
#define MASS_VOL__DATA_MANIPULATION_H

#include <msv/types/types.h>
#include <msv/types/vec3.h>


namespace massVolVis
{

class DataManipulation
{
public:
    /**
     *  Scales a block 2 times down
     */
    static void reduceTwice( void* dst, const void* src, const Vec3_ui32& srcDim, const byte bytes );

    /**
     *  Combines eight smaller blocks in to a bigger one
     */
    static void combineEight( void* dst, const void* src, const Vec3_ui32& srcDim, const byte bytes );

    /**
     *  Removes derivatives from the memory. only first quarter of the data remain useful
     */
    static void removeDerivatives( byte* src, const Vec3_ui32& srcDim );

    static void stripBorders( void* data, const Vec3_ui32& blockDim, byte borderDim, byte bytesNum );
    static void addBorders( void* dst, const void* src, const Vec3_ui32& blockDim, byte borderDim, byte bytesNum );

    static void copyData( void* dst, const void* src, const Vec3_ui32& srcDim,
                   const Vec3_i64&  dstStride, const Vec3_i64& srcStride, byte bytesNum );
};


}

#endif // MASS_VOL__DATA_MANIPULATION_H

