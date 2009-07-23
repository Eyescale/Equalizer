
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

/** 
 * @file plugins/compressor.h
 * 
 * The API to create runtime-loadable compression plugins.
 *
 * To implement a compression plugin, the following steps are to be taken:
 * <ul>
 *   <li>Create a new shared library named EqualizerCompressorNAME.dll (Win32),
 *       libeqCompressorNAME.dylib (Mac OS X) or libeqCompressorNAME.so
 *       (Linux).</li>
 *   <li>Define EQ_PLUGIN_API and then include eq/plugins/compressor.h (this
 *       header file).</li>
 *   <li>Implement all C functions from this header file. You can use the
 *       default Equalizer compressors in src/lib/compressor as a template.</li>
 *   <li>Put the library in the plugin search path (see
 *       eq::Global::getPluginDirectories(), defaults to EQ_PLUGIN_PATH or
 *       "/usr/local/share/Equalizer/plugins;.eqPlugins;$LD_LIBRARY_PATH".</li>
 *   <li>Run the image unit test (tests/image) to verify your plugin.</li>
 *   <li>Set the compression ratio and speed according to the output of the
 *       image unit test. Use the Equalizer RLE compressor as baseline.</li>
 *   <li>Request official names for your compressors.</li>
 * </ul>
 */

#ifndef EQ_PLUGINS_COMPRESSOR
#define EQ_PLUGINS_COMPRESSOR

/** @cond IGNORE */
#include <sys/types.h>

typedef unsigned long long eq_uint64_t;

#ifdef WIN32
#  ifdef EQ_PLUGIN_BUILD
#    define EQ_PLUGIN_API __declspec(dllexport) 
#  else
#    define EQ_PLUGIN_API __declspec(dllimport)
#  endif
#else // WIN32
#  define EQ_PLUGIN_API
#endif
/** @endcond */


#ifdef __cplusplus
extern "C"
{
#endif
 
    /** @name Compressor Plugin API Versioning */
    /*@{*/
    /** The version of the Compressor API described by this header. */
    #define EQ_COMPRESSOR_VERSION 1
    /** At least version 1 of the Compressor API is described by this header. */
    #define EQ_COMPRESSOR_VERSION_1 1
    /*@}*/

    /**
     * @name Compressor type name registry
     *
     * The compressor type registry ensures the uniqueness of compressor
     * names. It is maintained by the Equalizer development team
     * <info@equalizergraphics.com>. New types can be requested free of charge.
     */
    /*@{*/
    /** No Compressor */
    #define EQ_COMPRESSOR_NONE             0x1u
    /** RLE Compression of 4-byte tokens. */
    #define EQ_COMPRESSOR_RLE_UNSIGNED     0x2u
    /** RLE Compression of 1-byte tokens. */
    #define EQ_COMPRESSOR_RLE_BYTE         0x3u
    /** RLE Compression of three 1-byte tokens. */
    #define EQ_COMPRESSOR_RLE_3_BYTE       0x4u
    /** RLE Compression of four 1-byte tokens. */
    #define EQ_COMPRESSOR_RLE_4_BYTE       0x5u
    /** RLE Compression of four float32 tokens. */
    #define EQ_COMPRESSOR_RLE_4_FLOAT      0x6u
    /** RLE Compression of four float16 tokens. */
    #define EQ_COMPRESSOR_RLE_4_HALF_FLOAT 0x7u
    /** Differential RLE Compression of three 1-byte tokens. */
    #define EQ_COMPRESSOR_DIFF_RLE_3_BYTE  0x8u
    /** Differential RLE Compression of three 1-byte tokens. */
    #define EQ_COMPRESSOR_DIFF_RLE_4_BYTE  0x9u
    /** RLE Compression of one 4-byte token. */
    #define EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED  0xau

    /**
     * Private types -FOR DEVELOPMENT ONLY-.
     *
     * Any name equal or bigger than this can be used for in-house development
     * and testing. As soon as the Compressor DSO is distributed, request public
     * types free of charge from info@equalizergraphics.com.
     */
    #define EQ_COMPRESSOR_PRIVATE         0xefffffffu
    /*@}*/

    /**
     * @name Compressor token types
     *
     * The compressor token type is reported by the DSO, and defines which type
     * of input data can be processed by the given compressor. It is used by
     * Equalizer to select candidates for compression.
     */
    /*@{*/
    /** Data is processed in one-byte tokens. */
    #define EQ_COMPRESSOR_DATATYPE_BYTE       1
    /** Data is processed in four-byte tokens. */
    #define EQ_COMPRESSOR_DATATYPE_UNSIGNED   2
    /** Data is processed in float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_HALF_FLOAT 3
    /** Data is processed in float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_FLOAT      4


    /** Data is processed in three interleaved streams of one-byte tokens. */
    #define EQ_COMPRESSOR_DATATYPE_3_BYTE       1024
    /** Data is processed in four interleaved streams of one-byte tokens. */
    #define EQ_COMPRESSOR_DATATYPE_4_BYTE       1025
    /** Data is processed in four interleaved streams of float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_3_HALF_FLOAT 1026
    /** Data is processed in four interleaved streams of float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT 1027
    /** Data is processed in four interleaved streams of float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_3_FLOAT      1028
    /** Data is processed in four interleaved streams of float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_4_FLOAT      1029
    /**Data is processed in two interleaved streams, one 24 bit and one 8 bit.*/
    #define EQ_COMPRESSOR_DATATYPE_3BYTE_1BYTE  2048
    /*@}*/

    /**
     * @name Compressor capability flags
     *
     * Capability flags define what special features a compressor supports. They
     * are queried from the DSO, and passed as input to certain functions to
     * select a given mode.
     */
    /*@{*/
    /** 
     * The compressor can (query time) or should (compress) write the compressed
     * data in the same place as the uncompressed data.
     */
    #define EQ_COMPRESSOR_INPLACE    1
    /**
     * The compressor can handle linear data (query time), or the input data is
     * linear (compress, decompress). Typically used for binary data.
     */
    #define EQ_COMPRESSOR_DATA_1D    2
    /**
     * The compressor can handle two-dimensional data (query time), or the input
     * data is two-dimensional (compress, decompress). Typically used for image
     * data.
     */
    #define EQ_COMPRESSOR_DATA_2D    4
    /** 
     * The compressor can (query time) or should (compress) ignore the
     * most-significant element of the input data. Typically used for image data
     * when the alpha-channel is present in the input data, but unneeded.
     */
    #define EQ_COMPRESSOR_IGNORE_MSE 8
    /*@}*/

    /** @name DSO information interface. */
    /*@{*/
    /** Information about one compressor. */
    struct EqCompressorInfo
    {
        /**
         * The compressor API version used.
         *
         * Set on input to the API version used in Equalizer. Has to be set to
         * EQ_COMPRESSOR_VERSION on output to declare the API version used to
         * compile the DSO.
         */
        unsigned version;

        /** The type name of the compressor (output). */
        unsigned name;
        /** The token type supported by the compressor (output). */
        unsigned tokenType;
        /** Capabilities supported by the compressor (output). */
        eq_uint64_t capabilities;
        /** Compression quality (output, 1.0f: loss-less, <1.0f: lossy). */
        float quality;
        /** Approximate compression ratio (output, sizeCompressed/sizeIn). */
        float ratio;
        /** Approximate compression speed relative to BYTE_RLE (output). */
        float speed;
    };
    
    /** @return the number of compressors implemented in the DSO. */
    EQ_PLUGIN_API size_t EqCompressorGetNumCompressors();
    /** Query information of the nth compressor in the DSO. */
    EQ_PLUGIN_API void EqCompressorGetInfo( const size_t n,
                                            EqCompressorInfo* const info );
    /*@}*/

    /** @name Compressor lifecycle management. */
    /*@{*/
    /**
     * Instantiate a new compressor.
     *
     * This function has to create a new instance of the given compressor
     * type. Multiple instances might be used concurrently. One given instance
     * is always used from one thread at any given time.
     *
     * @param name the type name of the compressor.
     * @return an opaque pointer to the compressor instance.
     */
    EQ_PLUGIN_API void* EqCompressorNewCompressor( const unsigned name );
    /**
     * Release a compressor instance.
     *
     * @param compressor the compressor instance to free.
     */
    EQ_PLUGIN_API void EqCompressorDeleteCompressor( void* const compressor );

    /**
     * Instantiate a new decompressor.
     *
     * This function might create a new instance of the given decompressor
     * type. Multiple instances might be used concurrently. One given instance
     * is always used from one thread at any given time. State-less
     * decompressors might return 0.
     *
     * @param name the type name of the decompressor.
     * @return an opaque pointer to the decompressor instance, or 0 if no
     *         instance is needed by the implementation.
     */
    EQ_PLUGIN_API void* EqCompressorNewDecompressor( const unsigned name );
    /**
     * Release a decompressor instance.
     *
     * @param decompressor the decompressor instance to free.
     */
    EQ_PLUGIN_API void EqCompressorDeleteDecompressor(void* const decompressor);
    /*@}*/

    /** @name Compressor worker functions */
    /*@{*/
    /** 
     * Compress data.
     * 
     * The number of dimensions in the input and output data is given as a
     * flag. The input dimensions give an offset and a size for each dimension
     * in the format <code>dim0_offset, dim0_size, dim1_offset, ...,
     * dimN_size</code>. The offset does not apply to the input pointer, it is
     * merely a hint on where the data is positioned, e.g., where a 2D image is
     * positioned in a virtual framebuffer. The size of the input data is
     * <code>mul( inDims[1,3,...,n] ) * sizeof( info->dataType )</code>.<br>
     *
     * The compressor has to store the results internally in its instance data
     * The result of the compression run will be queried later. Results of
     * previous compression do not have to be retained, i.e., they can be
     * overwritten on subsequent compression runs.
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param in the pointer to the input data.
     * @param inDims the dimensions of the input data.
     * @param flags capability flags for the compression.
     */
    EQ_PLUGIN_API void EqCompressorCompress( void* const compressor, 
                                             const unsigned name,
                                             void* const in, 
                                             const eq_uint64_t* inDims,
                                             const eq_uint64_t flags );

    /** 
     * Return the number of results produced by the last compression.
     *
     * A compressor might generate multiple output stream, e.g., when operating
     * on structured data or using parallel compression routines.
     * 
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @return the number of output results.
     */
    EQ_PLUGIN_API unsigned EqCompressorGetNumResults( void* const compressor,
                                                      const unsigned name );

    /** 
     * Return the ith result of the last compression.
     * 
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param i the result index to return.
     * @param out the return value to store the result pointer.
     * @param outSize the return value to store the result size in bytes.
     */
    EQ_PLUGIN_API void EqCompressorGetResult( void* const compressor, 
                                              const unsigned name,
                                              const unsigned i, 
                                              void** const out, 
                                              eq_uint64_t* const outSize );

    /** 
     * Decompress data.
     * 
     * The decompressor gets all result pointers as produced by the compressor
     * as input. The routine should use the output buffer fully. For dimensions
     * and output size see EqCompressorCompress.
     *
     * @param decompressor the decompressor instance, can be 0.
     * @param name the type name of the decompressor.
     * @param in the pointer to an array of input data pointers.
     * @param inSizes the array of input data sizes in bytes.
     * @param numInputs the number of input data elements.
     * @param out the pointer to a pre-allocated buffer for the uncompressed
     *            output result.
     * @param outDims the dimensions of the output data.
     * @param flags capability flags for the decompression.
     * @sa EqCompressorCompress
     */
    EQ_PLUGIN_API void EqCompressorDecompress( void* const decompressor, 
                                               const unsigned name,
                                               const void* const* in, 
                                               const eq_uint64_t* const inSizes,
                                               const unsigned numInputs,
                                               void* const out,
                                               eq_uint64_t* const outDims,
                                               const eq_uint64_t flags );
    /*@}*/
#ifdef __cplusplus
}
#endif
#endif // EQ_PLUGINS_COMPRESSOR
