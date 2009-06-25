
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

#include "compressorRLE.h"

#include "../base/omp.cpp" // WAR: directly include eq code to avoid duplication

namespace eq
{
namespace plugin
{
void CompressorRLE::_setupResults( const uint32_t nChannels,
                                   const uint64_t inSize )
{
    // determine number of chunks and set up output data structure
#ifdef EQ_USE_OPENMP
    const size_t nChunks = nChannels * base::OMP::getNThreads() * 4;
#else
    const size_t nChunks = nChannels;
#endif

    while( _results.size() < nChunks )
        _results.push_back( new Result );

    const uint64_t maxChunkSize = (inSize/nChunks + 1) * 3;
    for( size_t i = 0; i < nChunks; ++i )
        _results[i]->resize( maxChunkSize );
}

}
}
