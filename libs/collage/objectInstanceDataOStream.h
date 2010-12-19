
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef CO_OBJECTINSTANCEDATAOSTREAM_H
#define CO_OBJECTINSTANCEDATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace co
{
    class Object;
    struct ObjectInstancePacket;

    /**
     * The DataOStream for object instance data.
     */
    class ObjectInstanceDataOStream : public ObjectDataOStream
    {
    public:
        ObjectInstanceDataOStream( const ObjectCM* cm );
        virtual ~ObjectInstanceDataOStream();
 
        void setInstanceID( const uint32_t instanceID )
            { _instanceID = instanceID; }

        void setNodeID( const NodeID& nodeID )
            { _nodeID = nodeID; }
        const NodeID& getNodeID() const { return _nodeID; }

    protected:
        virtual void sendData( const uint32_t name, const uint32_t nChunks,
                               const void* const* chunks, 
                               const uint64_t* chunkSizes,
                               const uint64_t sizeUncompressed );

        virtual void sendFooter( const uint32_t name, const uint32_t nChunks,
                                 const void* const* chunks, 
                                 const uint64_t* chunkSizes,
                                 const uint64_t sizeUncompressed );

    private:
        NodeID        _nodeID;
        uint32_t      _instanceID;

        void _sendPacket( ObjectInstancePacket& packet,
                          const uint32_t compressor, const uint32_t nChunks,
                          const void* const* chunks, const uint64_t* chunkSizes,
                          const uint64_t size );
    };
}
#endif //CO_OBJECTINSTANCEDATAOSTREAM_H
