
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef COBASE_CPUCOMPRESSOR_H
#define COBASE_CPUCOMPRESSOR_H

#include "compressor.h"

namespace co
{
namespace base
{
    /** @internal A C++ class to handle one CPU (de)compressor instance. */
    class CPUCompressor : public Compressor
    {
    public:
        /** Construct a new CPU-based compressor */
        CPUCompressor() {}
        virtual ~CPUCompressor() {}

        /**
         * Compress two-dimensional data.
         *
         * @param in the pointer to the input data. 
         * @param pvpIn the dimensions of the input data
         * @param flags capability flags for the compression
         */
        COBASE_API void compress( void* const in, 
                                 const uint64_t  pvpIn[4],
                                 const eq_uint64_t flags );

        /**
         * Compress one-dimensional data.
         *
         * @param in the pointer to the input data. 
         * @param inDims the dimensions of the input data
         */
        COBASE_API void compress( void* const in, const uint64_t inDims[2] );

        /** get the number of compressed chunks. */
        COBASE_API unsigned getNumResults() const;

        /**
         * get the compressed Data for the specified chunk 
         *
         * @param i the result index to return. 
         * @param out the return value to store the result pointer
         * @param outSize the return value to store the result size in bytes
         */
        COBASE_API void getResult( const unsigned i, 
                                     void** const out, 
                                     uint64_t* const outSize ) const ;

        /**
         * Decompress two-dimensional data.
         *
         * @param in the pointer to an array of input data pointers
         * @param inSizes the array of input data sizes in bytes
         * @param numInputs the number of input data elements
         * @param out the pointer to a pre-allocated buffer for the 
         *            uncompressed output result.
         * @param pvpOut the dimensions of the output data.
         * @param flags capability flags for the decompression.
         */
        COBASE_API void decompress( const void* const* in,
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs, void* const out,
                                    uint64_t pvpOut[4], const uint64_t flags );

        /**
         * Decompress one-dimensional data.
         *
         * @param in the pointer to an array of input data pointers
         * @param inSizes the array of input data sizes in bytes
         * @param numInputs the number of input data elements
         * @param out the pointer to a pre-allocated buffer for the 
         *            uncompressed output result.
         * @param outDim the dimensions of the output data.
         */
        COBASE_API void decompress( const void* const* in,
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs, void* const out,
                                    uint64_t outDim[2] );

        
        /**
         * Find the best compressor in all plugins for the given parameters.
         *
         * This convenience method searches all compressors in all plugins to
         * find the compressor which matches best the given parameters.
         *
         * @param tokenType the structure of the data to compress.
         * @param minQuality minimal quality of the compressed data, with 0 = no
         *                   quality and 1 = full quality, no loss.
         * @param ignoreMSE the most-significant element of each token can be
         *                  ignored, typically the alpha channel of an image.
         */
        static COBASE_API uint32_t chooseCompressor( 
                                                const uint32_t tokenType,
                                                const float minQuality = 1.0f,
                                                const bool ignoreMSE = false );

        /**
         * Find and init the best compressor in all plugins for the given
         * parameters.
         *
         * @param dataType the structure of the data to compress.
         * @param quality minimal quality of the compressed data, with 0 = no
         *                   quality and 1 = full quality, no loss.
         * @param ignoreMSE the most-significant element of each token can be
         *                  ignored, typically the alpha channel of an image.
         * @return true if a compressor was found, false otherwise.
         */
        COBASE_API bool initCompressor( const uint32_t dataType, 
                                        const float quality,
                                        const bool ignoreMSE = false );
    };
}
}
#endif  // COBASE_CPUCOMPRESSOR_H
