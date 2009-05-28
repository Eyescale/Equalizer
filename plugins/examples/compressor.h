/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#define EQ_PLUGIN

#include <eq/plugin/compressor.h>
#include <eq/base/base.h>
#include <eq/client/Compressor.h>

namespace eq
{
namespace plugin
{
                                                                                    
    typedef void  (*CompressorGetInfo_t)( EqCompressorInfo* const );
    typedef void* (*NewCompressor_t)();

    struct Functions
    {
        Functions();

        CompressorGetInfo_t      getInfo;
        NewCompressor_t          newCompressor;
    };
    

    typedef base::Bufferb Result;

    class Compressor
    {
    public:
        Compressor( const uint32_t numChannel );

        virtual ~Compressor(){}
        
        virtual void compress( void* const inData, 
                               const uint64_t inSize, 
                               const bool useAlpha ) = 0;

        virtual void decompress( const void** const inData, 
                                 const uint64_t* const inSizes, 
                                 void* const outData, 
                                 const uint64_t* const outSize )=0;


        std::vector< Result* >& getResults(){ return _results; }
        
        unsigned getName(){ return _name; }

    protected: 
      std::vector< Result* > _results;     //!< The pixel data
      unsigned _name;  
      const uint32_t _numChannels;
      bool _swizzleData;
      
        
    };                                                                                                                      
}
}

#endif
