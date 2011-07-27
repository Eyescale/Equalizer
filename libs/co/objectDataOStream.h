
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

    /** The DataOStream for object data. */
    class ObjectDataOStream : public DataOStream
    {
    public:
        ObjectDataOStream( const ObjectCM* cm );
        virtual ~ObjectDataOStream(){}
 
        virtual void reset();

        /** Set up commit of the given version to the receivers. */
        virtual void enableCommit( const uint128_t& version,
                                   const Nodes& receivers );

        uint128_t getVersion() const { return _version; }

    protected:
        void sendData( ObjectDataPacket& packet, const void* buffer,
                       const uint64_t size, const bool last );

        const ObjectCM* _cm;
        uint128_t _version;
        uint32_t _sequence;
    };
}
#endif //CO_OBJECTDATAOSTREAM_H
