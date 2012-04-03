
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "cpuCompressor.h" // used inline

namespace co
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
CO_API extern lunchbox::a_int32_t nBytesSaved;
CO_API extern lunchbox::a_int32_t nBytesSent;
#endif

    template< typename P >
    inline void DataOStream::sendPacket( P& packet, const void* buffer,
                                         const uint64_t size, const bool last )
    {
        EQASSERT( last || size != 0 );

        if( _connections.empty( ))
            return;

#ifdef EQ_INSTRUMENT_DATAOSTREAM
        nBytesSent += (size * long(_connections.size( )));
#endif
        packet.dataSize = size;
        packet.last = last;

        if( _compressorState == STATE_UNCOMPRESSED ||
            _compressorState == STATE_UNCOMPRESSIBLE )
        {
            EQASSERT( size == 0 || buffer );

            packet.compressorName = EQ_COMPRESSOR_NONE;
            packet.nChunks = 1;

            if( size == 0 )
                Connection::send( _connections, packet );
            else
                Connection::send( _connections, packet, buffer, size );
            return;
        }

        packet.nChunks = _compressor->getNumResults();
        uint64_t* chunkSizes = static_cast< uint64_t* >(
                                  alloca( packet.nChunks * sizeof( uint64_t )));
        void** chunks = static_cast< void ** >(
                            alloca( packet.nChunks * sizeof( void* )));
        

#ifdef EQ_INSTRUMENT_DATAOSTREAM
        const uint64_t compressedSize = _getCompressedData( chunks, chunkSizes);
        nBytesSaved += ((size - compressedSize) * long(_connections.size( )));
#else
        _getCompressedData( chunks, chunkSizes);
#endif
        packet.compressorName = _compressor->getName();
        Connection::send( _connections, packet, chunks, chunkSizes,
                          packet.nChunks );
    }
}
