
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQUTIL_COMPRESSORDATAGPU_H
#define EQUTIL_COMPRESSORDATAGPU_H

#include <eq/base/compressorData.h>
#include <eq/fabric/pixelViewport.h>

namespace eq
{
namespace util
{
    /** A C++ class to abstract a GPU compressor instance. */
    class CompressorDataGPU : public base::CompressorData
    {
    public:
        /** Construct a new compressorData */
        CompressorDataGPU( const GLEWContext* glewContext = 0 )
            : CompressorData()
            , _glewContext( glewContext ){}

        /** Set a valid glewContext */
        void setGLEWContext( const GLEWContext* glewContext )
            { _glewContext = glewContext; }

        /**
         * Determine if the downloader is valid
         *
         * @param inputToken the input token type
         */
        bool isValidDownloader( const uint32_t inputToken ) const;

        /**
         * Determine if the uploader is valid
         *
         * @param inputToken the input token type.
         * @param outputToken the output token produced 
         */
        bool isValidUploader( const uint32_t inputToken,
                              const uint32_t outputToken ) const;

        /**
         * Find and init a downloader for the given quality and token.
         *
         * If the uploader found is the same as the current uploader, no change
         * will be made.  If no uploader is found, the current uploader instance
         * (if any) is freed.
         *
         * @param minQuality the minimum quality.
         * @param tokenType the token produced
         **/
        void initDownloader( const float minQuality, const uint32_t tokenType );

        /**
         * Init a named downloader.
         *
         * @param name downloader name
         **/
        bool initDownloader( const uint32_t name );

        /**
         * Find and init an uploader which wil be compatible with the
         * specified input and output token type. 
         *
         * If the uploader found is the same that the current uploader,
         * no change will be make. 
         * If no uploader found, a reset of the instance data will be 
         * perform.
         *
         * @param gpuTokenType  the input token type of the data.
         * @param tokenType the output token type of the data.  
         **/
        void initUploader( const uint32_t gpuTokenType,
                           const uint32_t tokenType );

        /**
         * Download data from the frame buffer or texture to cpu
         *
         * @param pvpIn the dimensions of the input data
         * @param source texture name to process.
         * @param flags capability flags for the compression
         * @param pvpOut the dimensions of the output data
         * @param out the pointer to the output data
         */
        void download( const fabric::PixelViewport& pvpIn,
                       const unsigned     source,
                       const eq_uint64_t  flags,
                       fabric::PixelViewport& pvpOut,
                       void**             out );

        /**
         * Upload data from cpu to the frame buffer or texture 
         *
         * @param buffer data source
         * @param pvpIn the dimensions of the input data 
         * @param flags capability flags for the compression
         * @param pvpOut the dimensions of the output data
         * @param destination the destination texture name.
         */
        void upload( const void*          buffer,
                     const fabric::PixelViewport& pvpIn,
                     const eq_uint64_t    flags,
                     const fabric::PixelViewport& pvpOut,  
                     const unsigned  destination = 0 );

        /**
         * Get the token type produced by a donwloader or accepted by the
         * uploader.
         **/
        uint32_t getExternalFormat() const { return _info->outputTokenType; }

        /**
         * Get the token type accepted by a donwloader or 
         * produced by the uploader.
         **/
        uint32_t getInternalFormat() const { return _info->tokenType; }

        /**
         * Get the token size produced by a downloader or consumed by an
         * uploader.
         */
        uint32_t getTokenSize() const { return _info->outputTokenSize; }

        /**
         * Get the downloader/uploader internal format corresponding to 
         * an OpenGL token type
         *
         * @param format the GL format 
         * @param type the GL typedata source
         */
        static EQ_EXPORT uint32_t getExternalFormat( const uint32_t format,
                                                     const uint32_t type );

        /** @return true if the actual compressor is able to ignore alpha */
        bool ignoreAlpha() const
            { return _info->capabilities & EQ_COMPRESSOR_IGNORE_MSE; }

        /**
         * add info to the outInfos vector about transerers which are compatible
         * with the quality, internalFormat, externalFormat and glewContext
         *
         * @param outInfos the info vector where the info are put
         * @param minQuality the minimum quality require.
         * @param internalFormat if the internal format is 0, the choice don't 
         *                       use this property for the selection of the
         *                       transferer.
         * @param externalFormat if the external format is 0, the choice don't 
         *                       use this property for the selection of the
         *                       transferer.
         * @param glewContext a valid glewContext. 
         */
        static EQ_EXPORT void addTransfererInfos(
                               base::CompressorInfos& outInfos,
                               const float minQuality, 
                               const uint32_t internalFormat,
                               const uint32_t externalFormat,
                               const GLEWContext* glewContext );
        /**
         * Get the opengl internal format corresponding to compressor data type
         *
         * @param internalFormat the compressor internalFormat
         */
        static EQ_EXPORT uint32_t getGLInternalFormat( 
                                               const uint32_t internalFormat );
 
        /**
         * Get the opengl external format corresponding to compressor data type
         *
         * @param externalFormat the compressor externalFormat
         */
        static EQ_EXPORT uint32_t getGLFormat( const uint32_t externalFormat );

        /**
         * Get the opengl external type corresponding to compressor data type
         *
         * @param externalFormat the compressor externalFormat
         */
        static EQ_EXPORT uint32_t getGLType( const uint32_t externalFormat );

    private:
        /** the initialized GLEW context describing corresponding
            to the current OpenGL context. */
        const GLEWContext* _glewContext;

    };
}
}
#endif  // EQUTIL_COMPRESSORDATACPU_H
