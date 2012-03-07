
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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
 * The image compositing pipeline in Equalizer uses two types of plugins:
 * transfer engines and CPU compressors. A transfer engine downloads and uploads
 * the data from the GPU to main memory. A CPU compressor compresses and
 * decompresses the data produced by a transfer engine. The chain of operations
 * for an image transfer is:
 *  -# Select and instantiate transfer compressor for image to download
 *  -# Run the download operation from the render thread
 *  -# Select and instantiate a CPU compressor based on the transfer output
 *     token type
 *  -# Run the compress operation from the transmission thread
 *  -# Send the compressed data results to the receiving node(s)
 *  -# Instantiate a CPU decompressor
 *  -# Run the decompressor from the command thread
 *  -# Select and instantiate a transfer compressor based on the CPU
 *     decompressor token type
 *  -# Run the upload operation on each render thread
 *
 * The operations 3 to 7 are omitted if the input and output frame are on the
 * same node. The operations 3 to 7 are replaced by transmitting the output data
 * of the download operation if no matching CPU compressor is found. Plugin
 * instances are cached and reused for subsequent operations from the same
 * thread.
 *
 * <img src="http://www.equalizergraphics.com/documents/design/images/imageCompression.png">
 *
 * To implement a compression plugin, the following steps are to be taken:
 *  - Create a new shared library named EqualizerCompressorNAME.dll (Win32),
 *    libEqualizerCompressorNAME.dylib (Mac OS X) or 
 *    libEqualizerCompressorNAME.so (Linux).
 *  - Define EQ_PLUGIN_BUILD and then include eq/plugins/compressor.h (this
 *    header file).
 *  - Implement all relevant C functions from this header file. All plugins have
 *    to implement EqCompressorGetNumCompressors, EqCompressorGetInfo,
 *    EqCompressorNewCompressor, EqCompressorNewDecompressor,
 *    EqCompressorDeleteCompressor and EqCompressorDeleteDecompressor. In
 *    addition, CPU compressors have to implement EqCompressorCompress,
 *    EqCompressorDecompress, EqCompressorGetNumResults and
 *    EqCompressorGetResult. Transfer plugins have to additionally implement
 *    EqCompressorIsCompatible, EqCompressorDownload and EqCompressorUpload.
 *    The default Equalizer compressors in src/lib/compressor may be used as a
 *    template.
 *  - Put the library in the plugin search path (see
 *    eq::Global::getPluginDirectories(), defaults to EQ_PLUGIN_PATH or
 *    "/usr/local/share/Equalizer/plugins;.eqPlugins;$HOME/.eqPlugins;$PWD;$LD_LIBRARY_PATH".
 *  - Run the image unit test (tests/image) to verify your plugin.
 *  - Set the compression ratio and speed according to the output of the
 *    image unit test. Use the Equalizer RLE compressor as baseline.
 *  - Request official names for your compressors.
 *
 * @sa plugins/compressorTypes.h, plugins/compressorTokens.h
 *
 * <h2>Changes</h2>
 * Version 3
 *  - Added GPU-based compression during upload and download:
 *    - Added functions: EqCompressorIsCompatible, EqCompressorDownload,
 *      EqCompressorUpload
 *    - Added members in EqCompressorInfo: outputTokenType, outputTokenSize
 *    - Added flags: EQ_COMPRESSOR_CPU, EQ_COMPRESSOR_TRANSFER,
 *      EQ_COMPRESSOR_USE_TEXTURE_2D, EQ_COMPRESSOR_USE_TEXTURE_RECT,
 *      EQ_COMPRESSOR_USE_FRAMEBUFFER
 *    - Added data types: EQ_COMPRESSOR_DATATYPE_INVALID,
 *      EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_BYTE,
 *      EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_10_10_10_2,
 *      EQ_COMPRESSOR_DATATYPE_RGBA_HALF_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_RGBA_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_BYTE,
 *      EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_10_10_10_2,
 *      EQ_COMPRESSOR_DATATYPE_BGRA_HALF_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT,
 *      EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT_24_8_NV,
 *      EQ_COMPRESSOR_DATATYPE_RGB_UNSIGNED_BYTE,
 *      EQ_COMPRESSOR_DATATYPE_RGB_HALF_FLOAT, EQ_COMPRESSOR_DATATYPE_RGB_FLOAT,
 *      EQ_COMPRESSOR_DATATYPE_BGR_UNSIGNED_BYTE,
 *      EQ_COMPRESSOR_DATATYPE_BGR_HALF_FLOAT, EQ_COMPRESSOR_DATATYPE_BGR_FLOAT
 *    - Added compressor type names: EQ_COMPRESSOR_DIFF_RLE_YUVA_50P,
 *      EQ_COMPRESSOR_RLE_YUVA_50P, EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA,
 *      EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
 *      EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_RGB10A2,
 *      EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_BGR10A2,
 *      EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA16F,
 *      EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA32F,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA_25P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA_25P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA16F_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA16F_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUVA_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGB_TO_RGB, EQ_COMPRESSOR_TRANSFER_RGB_TO_BGR,
 *      EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB16F,
 *      EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR16F,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB32F,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR32F,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB_25P,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR_25P,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR16F_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB16F_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA_50P,
 *      EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA_50P,
 *      EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
 *      EQ_COMPRESSOR_DIFF_RLE_RGBA, EQ_COMPRESSOR_DIFF_RLE_BGRA,
 *      EQ_COMPRESSOR_DIFF_RLE_RGBA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DIFF_RLE_BGRA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DIFF_RLE_RGB10_A2, EQ_COMPRESSOR_DIFF_RLE_BGR10_A2,
 *      EQ_COMPRESSOR_DIFF_RLE_RGB, EQ_COMPRESSOR_DIFF_RLE_BGR,
 *      EQ_COMPRESSOR_DIFF_RLE_DEPTH_UNSIGNED_INT, EQ_COMPRESSOR_RLE_RGBA16F,
 *      EQ_COMPRESSOR_RLE_BGRA16F, EQ_COMPRESSOR_DIFF_RLE_RGBA16F,
 *      EQ_COMPRESSOR_DIFF_RLE_BGRA16F, EQ_COMPRESSOR_DIFF_RLE_565_RGBA,
 *      EQ_COMPRESSOR_DIFF_RLE_565_BGRA,
 *      EQ_COMPRESSOR_DIFF_RLE_565_RGBA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DIFF_RLE_565_BGRA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_DIFF_RLE_565_RGB10_A2,
 *      EQ_COMPRESSOR_DIFF_RLE_565_BGR10_A2, EQ_COMPRESSOR_RLE_RGBA,
 *      EQ_COMPRESSOR_RLE_BGRA, EQ_COMPRESSOR_RLE_RGBA_UINT_8_8_8_8_REV,
 *      EQ_COMPRESSOR_RLE_BGRA_UINT_8_8_8_8_REV, EQ_COMPRESSOR_RLE_RGB10_A2,
 *      EQ_COMPRESSOR_RLE_BGR10_A2, EQ_COMPRESSOR_RLE_RGB,
 *      EQ_COMPRESSOR_RLE_BGR, EQ_COMPRESSOR_RLE_DEPTH_UNSIGNED_INT,
 *      EQ_COMPRESSOR_AG_RTT_JPEG_HQ,
 *      EQ_COMPRESSOR_AG_RTT_JPEG_MQ,
 *      EQ_COMPRESSOR_AG_RTT_JPEG_LQ
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
#else // _WIN32
#  define EQ_PLUGIN_API __attribute__ ((visibility("default")))
#endif
/** @endcond */

/** @name Compressor Plugin API Versioning */
/*@{*/
/** The version of the Compressor API described by this header. */
#define EQ_COMPRESSOR_VERSION 4
/** At least version 1 of the Compressor API is described by this header. */
#define EQ_COMPRESSOR_VERSION_1 1
/**At least version 2 of the Compressor API is described by this header.*/
#define EQ_COMPRESSOR_VERSION_2 1
/**At least version 3 of the Compressor API is described by this header.*/
#define EQ_COMPRESSOR_VERSION_3 1
/**At least version 4 of the Compressor API is described by this header.*/
#define EQ_COMPRESSOR_VERSION_4 1
/*@}*/

#include <co/plugins/compressorTokens.h>
#include <co/plugins/compressorTypes.h>

#ifdef __cplusplus
#  include <vector>

extern "C" {
#endif
    /**
     * @name Compressor capability flags
     *
     * Capability flags define what special features a compressor supports. They
     * are queried from the DSO, and passed as input to certain functions to
     * select a given capability of the plugin.
     */
    /*@{*/
    /**
     * The compressor can (query time) or should (compress) write the compressed
     * data in the same place as the uncompressed data.
     */
    #define EQ_COMPRESSOR_INPLACE    0x1

    /**
     * The compressor can handle linear data (query time), or the input data is
     * linear (compress, decompress). Typically used for binary data.
     */
    #define EQ_COMPRESSOR_DATA_1D    0x2

    /**
     * The compressor can handle two-dimensional data (query time), or the input
     * data is two-dimensional (compress, decompress). Typically used for image
     * data.
     */
    #define EQ_COMPRESSOR_DATA_2D    0x4

    /**
     * The compressor can, does or should drop the alpha channel.
     *
     * The plugin sets this flag during information time to indicate that it
     * will drop the alpha channel (transfer plugins) or can drop the alpha
     * channel (compressor plugins).
     *
     * During download, the flag will always be set if it was set at query
     * time. During compression, it will be set only if the alpha channel should
     * be dropped. It will never be set for plugins which did not indicate this
     * capability.
     *
     * For compression plugins it is assumed that setting this flag improves the
     * compression ratio by 25 percent. For transfer plugins, it is assumed that
     * the ratio already includes the alpha reduction.
     */
    #define EQ_COMPRESSOR_IGNORE_ALPHA 0x8
    /** Deprecated */
    #define EQ_COMPRESSOR_IGNORE_MSE EQ_COMPRESSOR_IGNORE_ALPHA

    /**
     * The compressor is a CPU compressor.
     *
     * CPU compressors implement data compression and decompression of a block
     * of memory.
     */
    #define EQ_COMPRESSOR_CPU 0

    /**
     * The compressor is a CPU-GPU transfer engine.
     * GPU compressors implement the download from a GPU framebuffer or texture
     * to main memory, as well as the corresponding upload. During this
     * operation, compression might take place.
     */
    #define EQ_COMPRESSOR_TRANSFER 0x10

    /**
     * Capability to use a GL_TEXTURE_RECTANGLE_ARB texture as source or
     * destination. If set, the transfer engine can (query time) or shall
     * (compress time) use a rectangular texture as the source or destination
     * for its operations.
     */
    #define EQ_COMPRESSOR_USE_TEXTURE_RECT 0x20

    /**
     * Capability to use a GL_TEXTURE_2D texture as source or destination.
     * If set, the transfer engine can (query time) or shall (compress time) use
     * a 2D texture as the source or destination for its operations.
     */
    #define EQ_COMPRESSOR_USE_TEXTURE_2D 0x80

    /**
     * Capability to use the frame buffer as source or destination.
     * If set, the transfer engine can (query time) or shall (compress time) use
     * the frame buffer as the source or destination for its operations.
     */
    #define EQ_COMPRESSOR_USE_FRAMEBUFFER 0x40

    /**
     * Capability to use asynchronous downloads.
     * If set, the transfer engine will (query time) or shall (download time)
     * use asynchronous downloads.
     * @version 4
     */
    #define EQ_COMPRESSOR_USE_ASYNC_DOWNLOAD 0x100

#if 0 // Not implemented yet
    /**
     * Capability to use asynchronous uploads.
     * If set, the transfer engine will (query time) or shall (upload time)
     * use asynchronous uploads.
     * @version 4
     */
    #define EQ_COMPRESSOR_USE_ASYNC_UPLOAD 0x200
#endif
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
         * @version 1
         */
        unsigned version;

        /** The type name of the compressor. @version 1 */
        unsigned name;

        /**
         * The input token type supported by the compressor.
         *
         * The input token type describes the format of the input data for a
         * compressor or downloader and the format of the output data for the
         * decompressor or uploader of the same compressor.
         * @version 1
         */
        unsigned tokenType;

        /** Capabilities supported by the compressor. @version 1 */
        eq_uint64_t capabilities;

        /** Compression quality (1.0f: loss-less, <1.0f: lossy). @version 1 */
        float quality;

        /** Approximate compression ratio (sizeCompressed/sizeIn). @version 1 */
        float ratio;

        /** Approximate compression speed relative to BYTE_RLE. @version 1 */
        float speed;

        /**
         * The output token type of a plugin.
         *
         * The output token type describes the format of the data produced by a
         * downloader and consumed by the uploader of the same compressor.
         *
         * A CPU compressor might set the output token type if its decompressor
         * produces an output different from the input.
         *
         * If this parameter is set, outputTokenSize has to be set as well.
         * @version 3
         */
        unsigned outputTokenType;

        /** The size of one output token in bytes. @version 3 */
        unsigned outputTokenSize;
    };

    /** @return the number of compressors implemented in the DSO. @version 1 */
    EQ_PLUGIN_API size_t EqCompressorGetNumCompressors();

    /**
     * Query information of the nth compressor in the DSO.
     *
     * Plugins aiming to be backward-compatible, i.e., usable in older version
     * than the one compiled against, have to carefully check the provided
     * runtime version in info. If they implement features incompatible with
     * older Equalizer versions, e.g., a CPU compressor with an outputTokenType
     * different from tokenType, they either have to implement a compatibility
     * code path or to disable the compressor by setting the tokenType to
     * EQ_COMPRESSOR_DATATYPE_INVALID.
     *
     * @version 1
     */
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
     * @version 1
     */
    EQ_PLUGIN_API void* EqCompressorNewCompressor( const unsigned name );

    /**
     * Release a compressor or downloader instance.
     *
     * @param compressor the compressor instance to free.
     * @version 1
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
     * @version 1
     */
    EQ_PLUGIN_API void* EqCompressorNewDecompressor( const unsigned name );
    /**
     * Release a decompressor instance.
     *
     * @param decompressor the decompressor instance to free.
     * @version 1
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
     * The compressor has to store the results internally in its instance data.
     * The result of the compression run will be queried later. Results of
     * previous compression do not have to be retained, i.e., they can be
     * overwritten on subsequent compression runs.
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param in the pointer to the input data.
     * @param inDims the dimensions of the input data.
     * @param flags capability flags for the compression.
     * @version 1
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
     * @version 1
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
     * @version 1
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
     * @version 1
     * @note outDims should be const, which was an oversight
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
     * given OpenGL version and/or extension required by the transfer engine.
     * 
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @return true if the compressor is compatible with the environment.
     * @version 3
     */
    EQ_PLUGIN_API bool EqCompressorIsCompatible( const unsigned name,
                                               const GLEWContext* glewContext );

    /**
     * Transfer frame buffer data into main memory.
     * 
     * This function has to transfer the specified frame buffer region or
     * texture from the GPU memory into main memory. In the process, a
     * transformation (including compression) of the data may take place. The
     * result buffer has to be allocated by the compressor. The buffer has to be
     * valied until the next call to this function or the destruction of this
     * instance.
     *
     * The correct SystemWindow is current, e.g., the OpenGL context is current
     * and the frame buffer is bound correctly. The format and type of the input
     * frame buffer are determined indirectly by the information provided by the
     * plugin for the given compressor name, that is, the plugin has
     * pre-declared the frame buffer type it processes during
     * EqCompressorGetInfo().
     *
     * The OpenGL context has been setup using Compositor::setupAssemblyState()
     * using the channel's pvp. If the OpenGL state is modified by this
     * function, it has to reset it before leaving.
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
     *  - EQ_COMPRESSOR_IGNORE_ALPHA if the alpha value of a color buffer may
     *    be dropped during download
     *  - EQ_COMPRESSOR_USE_TEXTURE_2D if the source is a 2D texture ID
     *  - EQ_COMPRESSOR_USE_TEXTURE_RECT if the source is a rectangle texture ID
     *  - EQ_COMPRESSOR_USE_FRAMEBUFFER if the source is an OpenGL frame buffer
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name, if EQ_COMPRESSOR_USE_TEXTURE_2D or
     *               EQ_COMPRESSOR_USE_TEXTURE_RECT is set.
     * @param flags capability flags for the compression (see description).
     * @param outDims the dimensions of the output data (see description).
     * @param out the pointer to the output data.
     * @version 3
     */
    EQ_PLUGIN_API void EqCompressorDownload( void* const        compressor,
                                             const unsigned     name,
                                             const GLEWContext* glewContext,
                                             const eq_uint64_t  inDims[4],
                                             const unsigned     source,
                                             const eq_uint64_t  flags,
                                             eq_uint64_t        outDims[4],
                                             void**             out );

    /**
     * Start transferring frame buffer data into main memory.
     * 
     * When a plugin indicates that it supports asynchronous downloads during
     * query time, this function will be set by Equalizer versions supporting
     * async downloads. This function should then initiate the download using
     * the given input parameters, which behave exactly as described in
     * EqCompressorDownload. The operation will be completed from another thread
     * using EqCompressorFinishDownload().
     * 
     * Older Equalizer versions will call EqCompressorDownload(), which should
     * also be implemented by plugins using EqCompressorStartDownload() and
     * EqCompressorFinishDownload() to perform a synchronous readback.
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param source texture name, if EQ_COMPRESSOR_USE_TEXTURE_2D or
     *               EQ_COMPRESSOR_USE_TEXTURE_RECT is set.
     * @param flags capability flags for the compression (see description).
     * @version 4
     */
    EQ_PLUGIN_API void EqCompressorStartDownload( void* const        compressor,
                                                  const unsigned     name,
                                                 const GLEWContext* glewContext,
                                                  const eq_uint64_t  inDims[4],
                                                  const unsigned     source,
                                                  const eq_uint64_t  flags );

    /**
     * Finish transferring frame buffer data into main memory.
     * 
     * Finish an operation started using EqCompressorStartDownload(). The
     * correct SystemWindow is current, e.g., a shared OpenGL context is current
     * and the frame buffer is bound correctly. No OpenGL context setup is done.
     *
     * @param compressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context describing corresponding
     *                    to the current OpenGL context.
     * @param inDims the dimensions of the input data (x, w, y, h).
     * @param flags capability flags for the compression (see description).
     * @param outDims the dimensions of the output data (see description).
     * @param out the pointer to the output data.
     * @version 4
     */
    EQ_PLUGIN_API void EqCompressorFinishDownload( void* const compressor,
                                                   const unsigned     name,
                                                 const GLEWContext* glewContext,
                                                   const eq_uint64_t  inDims[4],
                                                   const eq_uint64_t  flags,
                                                   eq_uint64_t outDims[4],
                                                   void**             out );

    /**
     * Transfer data from main memory into GPU memory.
     * 
     * This function applies the inverse operation of EqCompressorDownload, that
     * is, it transfers the specified buffer into the GPU. It may apply a
     * transformation, including decompression, during its operation. At the
     * end, the result must be located in the frame buffer or the provided
     * texture.
     *
     * The correct OpenGL context is current. The texture is initialized to the
     * size provided by inDims and it is not bound. The OpenGL context has been
     * setup using Compositor::setupAssemblyState() using the channel pvp. If
     * the OpenGL state is modified by this function, it has to reset it before
     * leaving.
     *
     * The parameters buffer, inDims, flags will contain the same values as the
     * parameters out, outDims, flags of the corresponding
     * EqCompressorDownload() call. Please refer to the documentation of this
     * function for further information.
     *
     * Flags will always contain EQ_COMPRESSOR_DATA_2D, and may contain:
     *  - EQ_COMPRESSOR_IGNORE_ALPHA if the alpha value of a color buffer may
     *     be dropped during upload
     *  - EQ_COMPRESSOR_USE_TEXTURE_2D if the destination is a 2D texture ID
     *  - EQ_COMPRESSOR_USE_TEXTURE_RECT if the destination is a rectancle
     *    texture ID
     *  - EQ_COMPRESSOR_USE_FRAMEBUFFER if the destination is an OpenGL frame
     *     buffer
     *
     * @param decompressor the compressor instance.
     * @param name the type name of the compressor.
     * @param glewContext the initialized GLEW context corresponding to the
     *                    current OpenGL context.
     * @param buffer the input data.
     * @param inDims the dimension of the input data.
     * @param flags capability flags for the compression.
     * @param outDims the result data size in the frame buffer.
     * @param destination the destination texture name if
     *                    EQ_COMPRESSOR_USE_TEXTURE_2D or
     *                    EQ_COMPRESSOR_USE_TEXTURE_RECT is set.
     * @version 3
     */
    EQ_PLUGIN_API void EqCompressorUpload( void* const        decompressor,
                                           const unsigned     name,
                                           const GLEWContext* glewContext, 
                                           const void*        buffer,
                                           const eq_uint64_t  inDims[4],
                                           const eq_uint64_t  flags,
                                           const eq_uint64_t  outDims[4],  
                                           const unsigned     destination );
#if 0
    // TODO: add EqCompressorStart/FinishUpload and document operations and
    // parameters
#endif

    /*@}*/
#ifdef __cplusplus
}
#endif
#endif // EQ_PLUGINS_COMPRESSOR
