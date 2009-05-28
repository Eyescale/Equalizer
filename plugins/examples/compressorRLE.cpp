
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "compressorRLE.h"
namespace eq
{
namespace plugin
{
void CompressorRLE::_writeHeader( Result** results, 
                                  const Header& header )
{
    // write size
    for( uint32_t i = 0; i < _numChannels; ++i )
    {
        memcpy( results[i]->data, &header, sizeof( header ));
    }   
}

eq::plugin::CompressorRLE::Header CompressorRLE::_readHeader( const uint8_t* data8 )
{
    Header header (0, 0 ); 
    memcpy( &header, data8, sizeof( header ));
    return header;
}   

void CompressorRLE::_setupResults( const uint64_t inSize )
{
    // determine number of chunks and set up output data structure
#ifdef EQ_USE_OPENMP
    const ssize_t nChunks = _numChannels * base::OMP::getNThreads() * 4;
#else
    const size_t nChunks = _numChannels;
#endif

    const uint32_t maxChunkSize = (inSize/nChunks + 1) << 1;

    for( size_t i = 0; i < nChunks; ++i )
        _results[i]->resize( maxChunkSize );
}

}
}
