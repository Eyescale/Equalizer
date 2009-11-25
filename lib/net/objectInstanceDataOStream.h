
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTINSTANCEDATAOSTREAM_H
#define EQNET_OBJECTINSTANCEDATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace eq
{
namespace net
{
    class Object;
    struct ObjectInstancePacket;

    /**
     * The DataOStream for object instance data.
     */
    class ObjectInstanceDataOStream : public ObjectDataOStream
    {
    public:
        ObjectInstanceDataOStream( const Object* object );
        virtual ~ObjectInstanceDataOStream();
 
        void setInstanceID( const uint32_t instanceID )
            { _instanceID = instanceID; }

        void setNodeID( const NodeID& nodeID )
            { _nodeID = nodeID; }
        const NodeID& getNodeID() const { return _nodeID; }

    protected:
        virtual void sendHeader( const void* buffer, const uint64_t size )
            { sendBuffer( buffer, size ); }
        virtual void sendBuffer( const void* buffer, const uint64_t size );
        virtual void sendFooter( const void* buffer, const uint64_t size );
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { sendFooter( buffer, size ); }

    private:
        NodeID        _nodeID;
        uint32_t      _instanceID;

        void _sendPacket( ObjectInstancePacket& packet,
                          const void* buffer, const uint64_t size );
    };
}
}
#endif //EQNET_OBJECTINSTANCEDATAOSTREAM_H
