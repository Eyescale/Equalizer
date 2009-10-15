
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
 
#ifndef EQ_PLUGIN_COMPRESSOR
#define EQ_PLUGIN_COMPRESSOR 

#include <eq/plugins/compressor.h>

#include <eq/base/base.h>
#include <eq/base/buffer.h>

#include <vector>

/**
 * @file compressor/compressor.h
 * 
 * Compression plugin provided with Equalizer.
 */

namespace eq
{
namespace plugin
{
    class Compressor
    {
    public:
        typedef void  (*CompressorGetInfo_t)( EqCompressorInfo* const );
        typedef void* (*NewCompressor_t)();
        typedef void (*Decompress_t)( const void* const*, const 
                                      eq_uint64_t* const,
                                      const unsigned, void* const, 
                                      const eq_uint64_t, const bool );

        struct Functions
        {
            Functions();
            
            unsigned                 name;
            CompressorGetInfo_t      getInfo;
            NewCompressor_t          newCompressor;
            Decompress_t             decompress;
        };
    
        /**
         * Construct a new compressor.
         */
        Compressor();

        virtual ~Compressor();

        /**
         * compress Data.
         *
         * @param inData data to compress.
         * @param nPixels number data to compress.
         * @param useAlpha use alpha channel in compression.
         */
        virtual void compress( const void* const inData,
                               const eq_uint64_t nPixels, 
                               const bool useAlpha ) = 0;


        typedef eq::base::Bufferb Result;
        typedef std::vector< Result* > ResultVector;

        /** @return the vector containing the result data. */
        const ResultVector& getResults() const { return _results; }

        /** @return the number of result items produced. */
        unsigned getNResults() const { return _nResults; }

    protected: 
        ResultVector _results;  //!< The compressed data
        unsigned _nResults;     //!< Number of elements used in _results
    }; 
}
}

#endif // EQ_PLUGIN_COMPRESSOR

