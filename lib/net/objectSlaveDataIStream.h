
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTSLAVEDELTADATAISTREAM_H
#define EQNET_OBJECTSLAVEDELTADATAISTREAM_H

#include "objectDataIStream.h"   // base class

namespace eq
{
namespace net
{
    struct ObjectSlaveDeltaPacket;

    /**
     * The DataIStream for object slave delta data.
     */
    class ObjectSlaveDeltaDataIStream : public ObjectDataIStream
    {
    public:
        ObjectSlaveDeltaDataIStream() {}
        virtual ~ObjectSlaveDeltaDataIStream() {}

        virtual Type getType() const { return TYPE_DELTA; }

    protected:
        virtual bool getNextBuffer( uint32_t* compressor, uint32_t* nChunks,
                                    const void** chunkData, uint64_t* size )
            {
                return _getNextBuffer< ObjectSlaveDeltaPacket >(
                    CMD_OBJECT_SLAVE_DELTA, compressor, nChunks, chunkData,
                    size );
            }
    };
}
}
#endif //EQNET_OBJECTSLAVEDELTADATAISTREAM_H
