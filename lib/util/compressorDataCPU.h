
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

#ifndef EQUTIL_COMPRSSORDATACPU_H
#define EQUTIL_COMPRSSORDATACPU_H

#include "compressorData.h"

namespace eq
{
namespace util
{
    /** A C++ class to abstract a compressor instance */
    class CompressorDataCPU : public CompressorData
    {
    public:
        /** Construct a new compressorData */
        CompressorDataCPU( ){}

        /**
         * Compress data 
         *
         * @param in the pointer to the input data. 
         * @param pvpIn the dimensions of the input data
         * @param flags capability flags for the compression
         */
        void compress( void* const in, 
                       const fabric::PixelViewport& pvpIn,
                       const eq_uint64_t flags );

        /** get the number of chunks compressed */
        unsigned getNumResults( );

        /**
         * get the compressed Data for the specified chunk 
         *
         * @param i the result index to return. 
         * @param out the return value to store the result pointer
         * @param outSize the return value to store the result size in bytes
         */
        void getResult( const unsigned i, 
                        void** const out, 
                        eq_uint64_t* const outSize );

        /**
         * decompress Data
         *
         * @param in the pointer to an array of input data pointers
         * @param inSizes the array of input data sizes in bytes
         * @param numInputs the number of input data elements
         * @param out the pointer to a pre-allocated buffer for the 
         *            uncompressed output result.
         * @param pvpOut the dimensions of the output data.
         * @param flags capability flags for the decompression.
         */
        void decompress( const void* const* in, 
                         const eq_uint64_t* const inSizes,
                         const unsigned numInputs,
                         void* const out,
                         fabric::PixelViewport& pvpOut,
                         const eq_uint64_t flags );

        bool initCompressor( uint32_t name )
        { 
            _isCompressor = true;
            return _initCompressor( name );
        }

        bool initDecompressor( uint32_t name )
        { 
            _isCompressor = false;
            return _initDecompressor( name );
        }


    };
}
}
#endif  // EQUTIL_COMPRSSORDATACPU_H
