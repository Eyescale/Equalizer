/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef EQ_PLUGIN_INTERFACECOMPRESSOR
#define EQ_PLUGIN_INTERFACECOMPRESSOR

#include <sys/types.h>

typedef unsigned long long eq_uint64_t;

#define EQ_DSO_DLLEXPORT __declspec(dllexport) 

#ifdef __cplusplus
extern "C"
{
#endif
 
    // versioning
    #define EQ_COMPRESSOR_VERSION 1
    #define EQ_COMPRESSOR_VERSION_1 1

    // Compressor registry 
    #define EQ_COMPRESSOR_NONE             0x1u
    #define EQ_COMPRESSOR_RLE_UNSIGNED     0x2u
    #define EQ_COMPRESSOR_RLE_BYTE         0x3u
    #define EQ_COMPRESSOR_RLE_3_BYTE       0x4u
    #define EQ_COMPRESSOR_RLE_4_BYTE       0x5u
    #define EQ_COMPRESSOR_RLE_4_FLOAT      0x6u
    #define EQ_COMPRESSOR_RLE_4_HALF_FLOAT 0x7u
    #define EQ_COMPRESSOR_DIFF_RLE_3_BYTE  0x8u
    #define EQ_COMPRESSOR_DIFF_RLE_4_BYTE  0x9u


    // Private types -FOR DEVELOPMENT ONLY-. 
    // Request public type from info@equalizergraphics.com for deployment
    #define EQ_COMPRESSOR_PRIVATE         0xefffffffu


    struct EqCompressorInfo
    {
        // set to EQ_COMPRESSOR_VERSION on output
        unsigned version;    // in and out

        // set variables below for (in) version >= 1
        unsigned name; // EQ_COMPRESSOR_name
        unsigned dataType; // see below
        unsigned tokenType; // see below
        eq_uint64_t capabilities; // see below
        float quality; // 1.0f: loss-less, <1.0f: lossy
        float ratio;   // ~ compression ratio (sizeCompressed/sizeIn)
        float speed;   // ~ speed( EQ_COMPRESSOR_BYTE_RLE ) / speed( this )

        // set variables below for (in) version >= 2
        // there be future extensions
    };

    // Compressor token type
    //   single base types
    #define EQ_COMPRESSOR_DATATYPE_BYTE       1   // uint8_t
    #define EQ_COMPRESSOR_DATATYPE_UNSIGNED   2   // uint32_t
    #define EQ_COMPRESSOR_DATATYPE_HALF_FLOAT 3   // float16_t
    #define EQ_COMPRESSOR_DATATYPE_FLOAT      4   // float32_t
    //   vector types
    #define EQ_COMPRESSOR_DATATYPE_1_BYTE       1023 // uint8_t[3] (data)
    #define EQ_COMPRESSOR_DATATYPE_3_BYTE       1024 // uint8_t[3] (e.g. RGB)
    #define EQ_COMPRESSOR_DATATYPE_4_BYTE       1025 // uint8_t[4] (e.g. RGBA)
    #define EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT 1026 // float16_t[4] eg RGBA16f
    #define EQ_COMPRESSOR_DATATYPE_4_FLOAT      1027 // float32_t[4] eg RGBA32f
      //   special types
    #define EQ_COMPRESSOR_DATATYPE_3BYTE_1BYTE  2048 // e.g. DEPTH_STENCIL

    // Compressor capability flags
    #define EQ_COMPRESSOR_INPLACE    1
    #define EQ_COMPRESSOR_DATA_1D    2
    #define EQ_COMPRESSOR_DATA_2D    4
    #define EQ_COMPRESSOR_IGNORE_MSE 8

    // query interface
    EQ_DSO_DLLEXPORT size_t EqCompressorGetNumCompressors();
    EQ_DSO_DLLEXPORT void EqCompressorGetInfo( const size_t n,
                                           EqCompressorInfo* const info );

    // lifecycle management
    EQ_DSO_DLLEXPORT void* EqCompressorNewCompressor( const unsigned name );
    EQ_DSO_DLLEXPORT void EqCompressorDeleteCompressor( void* const compressor );

    // Note: 0 is allowed (for state-less decompressors)
    EQ_DSO_DLLEXPORT void* EqCompressorNewDecompressor( const unsigned name );
    EQ_DSO_DLLEXPORT void EqCompressorDeleteDecompressor( const unsigned name,
                                                    void* const decompressor );

    // worker functions
    EQ_DSO_DLLEXPORT void EqCompressorCompress( void* const compressor, 
                                    void* const in, const eq_uint64_t* inDims,
                                    const eq_uint64_t flags );
    EQ_DSO_DLLEXPORT unsigned EqCompressorGetNumResults( void* const compressor );
    EQ_DSO_DLLEXPORT void EqCompressorGetResult( void* const compressor, 
                                       const unsigned i, 
                                       void** const out, 
                                       eq_uint64_t* const outSize );

    EQ_DSO_DLLEXPORT void EqCompressorDecompress( 
                                              void* const decompressor, 
                                              const void** const in, 
                                              const eq_uint64_t* const inSizes,
                                              const unsigned numInputs,
                                              void* const out,
                                              eq_uint64_t* const outDims,
                                              const eq_uint64_t flags );
#ifdef __cplusplus
}
#endif
#endif