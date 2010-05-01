
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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
 *  - Create a new shared library named EqualizerCompressorNAME.dll (Win32),
 *    libeqCompressorNAME.dylib (Mac OS X) or libeqCompressorNAME.so
 *    (Linux).
 *  - Define EQ_PLUGIN_API and then include eq/plugins/compressor.h (this
 *    header file).
 *  - Implement all C functions from this header file. You can use the
 *    default Equalizer compressors in src/lib/compressor as a template.
 *  - Put the library in the plugin search path (see
 *    eq::Global::getPluginDirectories(), defaults to EQ_PLUGIN_PATH or
 *    "/usr/local/share/Equalizer/plugins;.eqPlugins;$LD_LIBRARY_PATH".
 *  - Run the image unit test (tests/image) to verify your plugin.
 *  - Set the compression ratio and speed according to the output of the
 *    image unit test. Use the Equalizer RLE compressor as baseline.
 *  - Request official names for your compressors.
 *
 * <h2>Changes</h2>
 * Version 3
 *  - Added capabilities for GPU-based compression during upload and download
 *    - Added data types:
 *        - EQ_COMPRESSOR_DATATYPE_BGRA_BYTE
 *        - EQ_COMPRESSOR_DATATYPE_BGRA_HALF
 *        - EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT
 *        - EQ_COMPRESSOR_DATATYPE_BGRA_10A2
 *        - EQ_COMPRESSOR_DATATYPE_YUV_50P
 *    - Added flags:
 *        - EQ_COMPRESSOR_CPU
 *        - EQ_COMPRESSOR_TRANSFER
 *        - EQ_COMPRESSOR_USE_TEXTURE
 *        - EQ_COMPRESSOR_USE_FRAMEBUFFER
 *    - Added compressor names:
 *        - EQ_COMPRESSOR_TRANSFER_YUV_COLOR_8_50P
 *        - EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_8
 *        - EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_32F
 *        - EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_16F
 *        - EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_10A2
 *        - EQ_COMPRESSOR_TRANSFER_1TO1_DEPTH_8
 *    - Added members in EqCompressorInfo:
 *        - unsigned outputTokenType
 *        - unsigned outputTokenSize
 *    - Added funtions:
 *        - EqCompressorIsCompatible
 *        - EqCompressorDownload
 *        - EqCompressorUpload
 *
 * Version 2
 *  - Added EQ_COMPRESSOR_DIFF_RLE_565 to type name registry
 *  - Added EQ_COMPRESSOR_DIFF_RLE_10A2 to type name registry
 *  - Added EQ_COMPRESSOR_DATATYPE_RGB10_A2 to token list
 *
 * Version 1
 *  - Initial Release
 */

#ifndef EQ_PLUGINS_COMPRESSOR
#define EQ_PLUGINS_COMPRESSOR

/** @cond IGNORE */
#include <sys/types.h>
struct GLEWContextStruct;
struct WGLEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;
typedef struct WGLEWContextStruct WGLEWContext;
typedef unsigned long long eq_uint64_t;

#ifdef _MSC_VER
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
    #define EQ_COMPRESSOR_VERSION 3
    /** At least version 1 of the Compressor API is described by this header. */
    #define EQ_COMPRESSOR_VERSION_1 1
    /**At least version 2 of the Compressor API is described by this header.*/
    #define EQ_COMPRESSOR_VERSION_2 1
    /**At least version 3 of the Compressor API is described by this header.*/
    #define EQ_COMPRESSOR_VERSION_3 1
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
    /** Differential RLE Compression of four 1-byte tokens. */
    #define EQ_COMPRESSOR_DIFF_RLE_4_BYTE  0x9u
    /** RLE Compression of one 4-byte token. */
    #define EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED  0xau
    /** Lossy Differential RLE Compression. */
    #define EQ_COMPRESSOR_DIFF_RLE_565     0xbu
    /** RLE Compression of three token of 10-bits and one toke of 2-bits */
    #define EQ_COMPRESSOR_DIFF_RLE_10A2    0xcu
    /** RLE Compression of four float16 tokens. */
    #define EQ_COMPRESSOR_DIFF_RLE_4_HALF_FLOAT 0xdu
    /** 50% YUV Compression of BGRA8 data. */
    #define EQ_COMPRESSOR_TRANSFER_YUV_COLOR_8_50P  0xfu
    /** No compression, only a GPU-CPU transfer for RGBA8*/
    #define EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_8     0x10u
    /** No compression, only a GPU-CPU transfer for RGBA32F*/
    #define EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_32F   0x11u
    /** No compression, only a GPU-CPU transfer for RGBA16F*/
    #define EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_16F   0x12u
    /** No compression, only a GPU-CPU transfer for RGB10A2*/
    #define EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_10A2  0x13u
    /** No compression, only a GPU-CPU transfer for DEPTH*/
    #define EQ_COMPRESSOR_TRANSFER_1TO1_DEPTH_8     0x14u
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
    /** Data is processed in four interleaved streams of BGRA color tokens. */
    #define EQ_COMPRESSOR_DATATYPE_BGRA_BYTE  EQ_COMPRESSOR_DATATYPE_4_BYTE
    /** Data is processed in four interleaved streams of float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_3_HALF_FLOAT 1026
    /** Data is processed in four interleaved streams of float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT 1027
    /** Data is processed in four interleaved streams of BGRA color of float16 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_BGRA_HALF  EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT
    /** Data is processed in four interleaved streams of float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_3_FLOAT      1028
    /** Data is processed in four interleaved streams of float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_4_FLOAT      1029
    /** Data is processed in four interleaved streams of BGRA float32 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT EQ_COMPRESSOR_DATATYPE_4_FLOAT  
    /**Data is processed in two interleaved streams, one 24 bit and one 8 bit.*/
    #define EQ_COMPRESSOR_DATATYPE_3BYTE_1BYTE  2048
    /** Data is processed in four interleaved streams of BGRA color of 10A2 tokens. */
    #define EQ_COMPRESSOR_DATATYPE_BGRA_10A2 EQ_COMPRESSOR_DATATYPE_3BYTE_1BYTE
    /** Data is processed in four interleaved streams of YUV components. 
        Special image format with reduced color components. */
    #define EQ_COMPRESSOR_DATATYPE_YUV_50P     2049

    /**
    * Data is processed in three 10-bit color tokens and one 2-bit alpha
    * token.
    */
    #define EQ_COMPRESSOR_DATATYPE_RGB10_A2     2049

    /**
     * Private token types -FOR DEVELOPMENT ONLY-.
     *
     * Any token type equal or bigger than this can be used for in-house
     * development and testing. As soon as the Compressor DSO is distributed,
     * request public types free of charge from info@equalizergraphics.com.
     */
    #define EQ_COMPRESSOR_DATATYPE_PRIVATE      0xefffffffu
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

    /** 
     * The compressor is a CPU compressor, that is, it implements compress and
     * decompress.
     */
    #define EQ_COMPRESSOR_CPU 0

    /** 
     * The compressor is a transfer compressor, that is, it implements download
     * and upload.
     */
    #define EQ_COMPRESSOR_TRANSFER 16 

    /** 
     * The transfer engine can (query time) or should (compress) use a texture
     * as source or destination for its operations.
     */
    #define EQ_COMPRESSOR_USE_TEXTURE 32
    /** 
     * The transfer engine can (query time) or should (compress) use the frame
     * buffer as source or destination for its operations.
     */
    #define EQ_COMPRESSOR_USE_FRAMEBUFFER 64

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
        /** The type of the data produced by the compressor (version >= 3 ). */
        unsigned outputTokenType;
        /** The size of one output token in bytes (version >= 3 ). */
        unsigned outputTokenSize;
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
     * Instantiate a new compressor or a new downloader.
     *
     * This function has to create a new instance of the given compressor
     * type. Multiple instances might be used concurrently. One given instance
     * is always used from one thread at any given time.
     *
     * For one given name, there can only be one given implementation of a
     * compressor or downloader. This type has been given by the plugin during
     * getInfo.
     *
     * @param name the type name of the compressor.
     * @return an opaque pointer to the compressor instance.
     */
    EQ_PLUGIN_API void* EqCompressorNewCompressor( const unsigned name );

    /**
     * Release a compressor or downloader instance.
     *
     * @param compressor the compressor instance to free.
     */
    EQ_PLUGIN_API void EqCompressorDeleteCompressor( void* const compressor );

    /**
     * Instantiate a new decompressor or a new uploader.
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

    /** @name CPU Compressor worker functions */
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

    /** @name Transfer engine worker functions */
    /*@{*/
    /**
     * Check if the compressor may be used with the current OpenGL context.
     *
     * The OpenGL context is current, and has not be modified by this
     * function. The given glewContext is an initialized GLEWContext
     * corresponding to the OpenGL context. Typically this function checks for a
     * given OpenGL version and/or extension.
     * 
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @return true if the compressor is compatible with the environment.
     */
    EQ_PLUGIN_API bool EqCompressorIsCompatible( const unsigned name,
                                                 GLEWContext* glewContext );

    /**
     * Transfer frame buffer data into main memory.
     * 
     * This function has to transfer the specified frame buffer region from the
     * GPU memory into main memory. In the process, a transformation (including
     * compression) of the data may take place. The result buffer has to be
     * allocated by the compressor. the buffer integrity is guarantee until the 
     * next download call or the destruction of the class instance
     *
     * The correct OpenGL context is current and the frame buffer is bound
     * correctly. The format and type of the input frame buffer are determined
     * indirectly by the information provided by the plugin for the given
     * compressor name, that is, the plugin has pre-declared the frame buffer
     * type it processes during EqCompressorGetInfo().
     *
     * The OpenGL context has been setup using Compositor::setupAssemblyState()
     * using ??? pvp. If the OpenGL state is modified by this function, it has
     * to reset it before leaving.
     *
     * The pointer and data size is returned using the out parameters. The
     * outDims parameter has the format <code>x, w, y, h</code>. If the
     * compressor produces an image (structured data), the outDims should be set
     * to a multiple of inDims. For unstructured data the values should be set
     * to <code>x = 0, w = num_elements, y = 0, h = 1</code>. The output pointer
     * has to be valid until the next call to this function using the same
     * compressor instance.
     *
     * Flags will always contain EQ_COMPRESSOR_DATA_2D, and may contain:
     * <ul>
     *   <li>EQ_COMPRESSOR_IGNORE_MSE if the alpha value of a color buffer may
     *     be dropped during download</li>
     *   <li>EQ_COMPRESSOR_USE_TEXTURE if the source is a 2D texture ID</li>
     *   <li>EQ_COMPRESSOR_USE_FRAMEBUFFER if the source is an OpenGL frame 
     *     buffer and the source value will be zero.</li>
     * </ul>
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name to process.
     * @param flags capability flags for the compression (see description).
     * @param outDims the dimensions of the output data (see description).
     * @param out the pointer to the output data.
     */
    EQ_PLUGIN_API void EqCompressorDownload( void* const        compressor,
                                             const unsigned     name,
                                             GLEWContext*       glewContext,
                                             const eq_uint64_t  inDims[4],
                                             const unsigned     source,
                                             const eq_uint64_t  flags,
                                             eq_uint64_t        outDims[4],
                                             void**             out );

    /**
     * Transfer data from main memory into GPU memory.
     * 
     * This function applies the inverse operation of EqCompressorDownload,
     * that is, it transfers the specified buffer into the GPU. It may apply a
     * transformation, including decompression, during its operation. At the
     * end, the result must be located in the provided texture.
     *
     * The correct OpenGL context is current. The texture is initialized to the
     * size provided by inDims and it is not bound. The OpenGL context has been
     * setup using Compositor::setupAssemblyState() using ??? pvp. If the OpenGL
     * state is modified by this function, it has to reset it before leaving.
     *
     * The parameters buffer, inDims, flags will contain the same values as the
     * parameters out, outDims, flags of the corresponding
     * EqCompressorDownload() call. Please refer to the documentation of this
     * function for further information.
     *
     * Flags will always contain EQ_COMPRESSOR_DATA_2D, and may contain:
     * <ul>
     *   <li>EQ_COMPRESSOR_IGNORE_MSE if the alpha value of a color buffer may
     *     be dropped during upload</li>
     *   <li>EQ_COMPRESSOR_USE_TEXTURE if the destination is a 2D texture ID</li>
     *   <li>EQ_COMPRESSOR_USE_FRAMEBUFFER if the destination is an OpenGL frame 
     *     buffer and the destination value will be zero.</li>
     * </ul>
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @param buffer the datas input.
     * @param inDims the dimension of data in the frame buffer.
     * @param flags capability flags for the compression.
     * @param outDims the result data size
     * @param destination the destination texture name.
     */
    EQ_PLUGIN_API void EqCompressorUpload( void* const        decompressor,
                                           const unsigned     name,
                                           GLEWContext*       glewContext, 
                                           const void*        buffer,
                                           const eq_uint64_t  inDims[4],
                                           const eq_uint64_t  flags,
                                           const eq_uint64_t  outDims[4],  
                                           const unsigned     destination );
    /*@}*/
#ifdef __cplusplus
}
#endif
#endif // EQ_PLUGINS_COMPRESSOR
