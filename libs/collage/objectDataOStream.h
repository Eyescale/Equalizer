
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_OBJECTDATAOSTREAM_H
#define CO_OBJECTDATAOSTREAM_H

#include <co/dataOStream.h>   // base class
#include <co/version.h>       // enum

namespace co
{
    struct ObjectDataPacket;

    /**
     * The DataOStream for object data.
     */
    class ObjectDataOStream : public DataOStream
    {
    public:
        ObjectDataOStream( const ObjectCM* cm )
                : _cm( cm ), _version( VERSION_NONE )
                , _sequence( 0 ) {}

        virtual ~ObjectDataOStream(){}
 
        void setVersion( const uint128_t& version ) { _version = version; }
        uint128_t getVersion() const { return _version; }
        virtual void reset() { DataOStream::reset(); _sequence = 0; }

    protected:
        void sendPacket( ObjectDataPacket& packet, const uint32_t compressor,
                         const uint32_t nChunks, const void* const* chunks,
                         const uint64_t* chunkSizes, const uint64_t size );

        const ObjectCM* _cm;
        uint128_t _version;
        uint32_t _sequence;
    };
}
#endif //CO_OBJECTDATAOSTREAM_H
