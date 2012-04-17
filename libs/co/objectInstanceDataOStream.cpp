
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#include "objectInstanceDataOStream.h"

#include "log.h"
#include "masterCM.h"
#include "nodePackets.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectPackets.h"

namespace co
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const ObjectCM* cm )
        : ObjectDataOStream( cm )
        , _instanceID( EQ_INSTANCE_ALL )
        , _command( 0 )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::reset()
{
    ObjectDataOStream::reset();
    _nodeID = NodeID::ZERO;
    _instanceID = EQ_INSTANCE_NONE;
    _command = 0;
}

void ObjectInstanceDataOStream::enableCommit( const uint128_t& version,
                                              const Nodes& receivers )
{
    _command = CMD_NODE_OBJECT_INSTANCE_COMMIT;
    _nodeID = NodeID::ZERO;
    _instanceID = EQ_INSTANCE_NONE;
    ObjectDataOStream::enableCommit( version, receivers );
}

void ObjectInstanceDataOStream::enablePush( const uint128_t& version,
                                            const Nodes& receivers )
{
    _command = CMD_NODE_OBJECT_INSTANCE_PUSH;
    _nodeID = NodeID::ZERO;
    _instanceID = EQ_INSTANCE_NONE;
    ObjectDataOStream::enableCommit( version, receivers );
}

void ObjectInstanceDataOStream::push( const Nodes& receivers,
                                      const uint128_t& objectID,
                                      const uint128_t& groupID,
                                      const uint128_t& typeID )
{
    _command = CMD_NODE_OBJECT_INSTANCE_PUSH;
    _nodeID = NodeID::ZERO;
    _instanceID = EQ_INSTANCE_NONE;
    _setupConnections( receivers );

    _resend();
    NodeObjectPushPacket packet( objectID, groupID, typeID );
    _send( packet );

    _clearConnections();
}

void ObjectInstanceDataOStream::sendInstanceData( const Nodes& receivers )
{
    _command = CMD_NODE_OBJECT_INSTANCE;
    _nodeID = NodeID::ZERO;
    _instanceID = EQ_INSTANCE_NONE;
    _setupConnections( receivers );
    _resend();
    _clearConnections();
}

void ObjectInstanceDataOStream::sendMapData( NodePtr node,
                                             const uint32_t instanceID )
{
    _command = CMD_NODE_OBJECT_INSTANCE_MAP;
    _nodeID = node->getNodeID();
    _instanceID = instanceID;
    _setupConnection( node, true /* useMulticast */ );
    _resend();
    _clearConnections();
}

void ObjectInstanceDataOStream::enableMap( const uint128_t& version,
                                           NodePtr node,
                                           const uint32_t instanceID )
{
    _command = CMD_NODE_OBJECT_INSTANCE_MAP;
    _nodeID = node->getNodeID();
    _instanceID = instanceID;
    _version = version;
    _setupConnection( node, true /* useMulticast */ );
    _enable();
}

void ObjectInstanceDataOStream::sendData( const void* buffer,
                                          const uint64_t size, const bool last )
{
    LBASSERT( _command );

    ObjectInstancePacket packet( _nodeID, _cm->getObject()->getInstanceID( ));
    packet.command = _command;
    packet.instanceID = _instanceID;

    ObjectDataOStream::sendData( packet, buffer, size, last );
}

}
