
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
        CompressorDataGPU( GLEWContext* glewContext = 0 )
            : CompressorData()
            , _glewContext( glewContext ){}

        /** Set a valid glewContext */
        void setGLEWContext( GLEWContext* glewContext )
            { _glewContext = glewContext; }

        /**
         * Determine if the downloader is valid
         *
         * @param inputToken the input token type
         */
        bool isValidDownloader( uint32_t inputToken );

        /**
         * Determine if the uploader is valid
         *
         * @param inputToken the input token type.
         * @param outputToken the output token produced 
         */
        bool isValidUploader( uint32_t inputToken, uint32_t outputToken );

        /**
         * Found and init a downloader for the given quality 
         * and tokentype.
         * If the uploader found is the same that the current uploader,
         * no change will be make. 
         * If no uploader found, a reset of the instance data will be 
         * perform.
         **/
        void initDownloader( float minQuality, uint32_t tokenType );

        bool initDownloader( uint64_t name );
        /**
         * Find and init an uploader which wil be compatible with the
         * specified input and output token type. 
         *
         * If the uploader found is the same that the current uploader,
         * no change will be make. 
         * If no uploader found, a reset of the instance data will be 
         * perform.
         *
         * @param inTokenType  the input token type of the data.
         * @param outTokenType the output token type of the data.  
         **/
        void initUploader( uint32_t gpuTokenType, uint32_t tokenType );

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
         * Get the token type produced by a donwloader or 
         * accepted by the uploader.
         **/
        uint32_t getTokenType() const { return _info.outputTokenType; }

        /**
         * Get the token size produced by a donwloader or 
         * accepted by the uploader.
         **/
        uint32_t getTokenSize() const { return _info.outputTokenSize; }

        /**
         * Get the downloader/uploader internal format corresponding to 
         * an opengl token type
         *
         * @param format the GL format 
         * @param type the GL typedata source
         */
        static EQ_EXPORT uint32_t getTokenFormat( uint32_t format, uint32_t type  );

        /**
         * Get the size of one pixel
         *
         * @param type the GL typedata source
         */
        static EQ_EXPORT uint32_t getPixelSize( uint64_t pixelFormat );

        /* get if the actual compressor is able to ignore alpha */
        bool ignoreAlpha(){ return _info.capabilities & 
                            EQ_COMPRESSOR_IGNORE_MSE; }

        /**
         * add info to the outInfos vector about transerers which are compatible
         * with the qualita, tokentype and glewCotext
         *
         * @param outInfos the info vector where the info are put
         * @param minQuality the minimum quality require.
         * @param tokenType the input token type 
         * @param GLEWContext a valid glewContext 
         */
        static EQ_EXPORT void addTransfererInfos(
                               eq::base::CompressorInfos& outInfos,
                               float minQuality, 
                               uint32_t tokenType, 
                               GLEWContext* glewContext );
    private:
        /** the initialized GLEW context describing corresponding
            to the current OpenGL context. */
        GLEWContext* _glewContext;

    };
}
}
#endif  // EQUTIL_COMPRESSORDATACPU_H
