
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

#include "../base/compressorData.h" // base class
#include <eq/fabric/types.h>

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
         * Determine if the downloader is valid.
         *
         * @param internalFormat the input token type to the downloader.
         * @param ignoreAlpha true if the downloader may drop the alpha channel.
         */
        bool isValidDownloader( const uint32_t internalFormat,
                                const bool ignoreAlpha ) const;

        /**
         * Determine if the uploader is valid
         *
         * @param externalFormat the input to the uploader.
         * @param internalFormat the output of the uploader. 
         */
        bool isValidUploader( const uint32_t externalFormat,
                              const uint32_t internalFormat ) const;

        /**
         * Find and init a downloader for the given quality and token.
         *
         * If the uploader found is the same as the current uploader, no change
         * will be made.  If no uploader is found, the current uploader instance
         * (if any) is freed.
         *
         * @param internalFormat the input token type to the downloader.
         * @param minQuality the minimum quality.
         * @param ignoreAlpha true if the downloader may drop the alpha channel.
         */
        void initDownloader( const uint32_t internalFormat,
                             const float minQuality, const bool ignoreAlpha );

        /**
         * Init a named downloader.
         *
         * @param name downloader name
         */
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
         * @param externalFormat the input to the uploader.
         * @param internalFormat the output of the uploader. 
         */
        void initUploader( const uint32_t externalFormat,
                           const uint32_t internalFormat );

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

        /** @return true if the current downloader does not drop alpha. */
        bool hasAlpha() const
            { return (_info->capabilities & EQ_COMPRESSOR_IGNORE_ALPHA) == 0; }

        /**
         * Find all transfer plugins which comply with to the given parameters.
         *
         * @param internalFormat consider only plugins with this tokenType, if
         *                       set to EQ_COMPRESSOR_DATATYPE_NONE consider
         *                       all.
         * @param externalFormat consider only plugins with this outpuTokentype,
                                 if set to EQ_COMPRESSOR_DATATYPE_NONE consider
         *                       all.
         * @param minQuality the minimum required quality.
         * @param ignoreAlpha true if the downloader may drop the alpha channel.
         * @param glewContext a valid glewContext. 
         * @param result the output result vector.
         */
        static EQ_EXPORT void findTransferers( const uint32_t internalFormat,
                                               const uint32_t externalFormat,
                                               const float minQuality,
                                               const bool ignoreAlpha,
                                               const GLEWContext* glewContext,
                                               base::CompressorInfos& result );

    private:
        /** the initialized GLEW context describing corresponding
            to the current OpenGL context. */
        const GLEWContext* _glewContext;

    };
}
}
#endif  // EQUTIL_COMPRESSORDATACPU_H
