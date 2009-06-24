
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
 
#ifndef EQ_COMPRESSOR_COMPRESSOR
#define EQ_COMPRESSOR_COMPRESSOR 

#define EQ_PLUGIN_BUILD
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
    typedef void  (*CompressorGetInfo_t)( EqCompressorInfo* const );
    typedef void* (*NewCompressor_t)();
    typedef void (*Decompress_t)( const void* const*, const uint64_t* const,
                                  const unsigned, void* const, const uint64_t,
                                  const bool );

    struct Functions
    {
        Functions();

        unsigned                 name;
        CompressorGetInfo_t      getInfo;
        NewCompressor_t          newCompressor;
        Decompress_t             decompress;
    };
    

    typedef eq::base::Bufferb Result;

     /**
     * An interace for compressor / uncompressor data
     *
     */
    class Compressor
    {
    public:
        /**
         * Construct a new compressor.
         */
        Compressor();

        virtual ~Compressor();

        /** @name compress */
        /*@{*/
        /**
         * compress Data.
         *
         * @param inData data to compress.
         * @param nPixels number data to compress.
         * @param useAlpha use alpha channel in compression.
         */
        virtual void compress( const void* const inData,
                               const uint64_t nPixels, 
                               const bool useAlpha ) = 0;


        /** @name getResults */
        /*@{*/
        /**
         * get the number results that compression use to save data
         */
        const std::vector< Result* >& getResults() const { return _results; }

    protected: 
        std::vector< Result* > _results;  //!< The compressed data
    }; 
}
}

#endif // EQ_COMPRESSOR_COMPRESSOR

