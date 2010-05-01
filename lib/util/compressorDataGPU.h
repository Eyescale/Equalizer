
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

#ifndef EQUTIL_COMPRSSORDATAGPU_H
#define EQUTIL_COMPRSSORDATAGPU_H

#include "compressorData.h"

namespace eq
{
namespace util
{
    /** A C++ class to abstract a compressor instance */
    class CompressorDataGPU : public CompressorData
    {
    public:
        /** Construct a new compressorData */
        CompressorDataGPU( GLEWContext* glewContext = 0 )
            : CompressorData()
            , _glewContext( glewContext ){}

		/** Set a valid glewContext */
        void setGlewContext( GLEWContext* glewContext )
            { _glewContext = glewContext; }

		/**
         * determine if the downloader is valid
         *
         * @param inputToken the input token type.
         * @param outputToken the output token produced 
         */
        bool isValidDownloader( uint32_t name,
                                uint32_t inputToken, 
                                uint32_t outputToken,
                                float    minQuality );

		/**
         * determine if the uploader is valid
         *
         * @param inputToken the input token type.
         * @param outputToken the output token produced 
         */
        bool isValidUploader( uint32_t inputToken, 
                              uint32_t outputToken );

		/**
         * found and init an uploader for the given quality 
		 * and tokentype
         *
         * @param minQuality the minimum quality.
         * @param tokenType the token type of the data 
         */		
        void initUploader( uint32_t gpuTokenType, 
                           uint32_t tokenType );

		/**
         * found and init a downloader for the given quality 
		 * and tokentype
         *
         * @param minQuality the minimum quality.
         * @param tokenType the token type of the data 
         */
        void initDownloader( float minQuality, 
                             uint32_t tokenType );
		/**
         * Download data from the frame buffer or texture to cpu
         *
         * @param glewContext the initialized GLEW context 
         *                    corresponding to the current OpenGL context.
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

    private:
        /** the initialized GLEW context describing corresponding
            to the current OpenGL context. */
        GLEWContext* _glewContext;

    };
}
}
#endif  // EQUTIL_COMPRSSORDATACPU_H
