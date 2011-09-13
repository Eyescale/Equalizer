
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "nodePackets.h"
#include "object.h"
#include "objectPackets.h"


namespace co
{
StaticMasterCM::StaticMasterCM( Object* object ) 
        : ObjectCM( object )
{}

StaticMasterCM::~StaticMasterCM(){}

void StaticMasterCM::addSlave( Command& command,
                               NodeMapObjectReplyPacket& reply )
{
    EQASSERT( command->type == PACKETTYPE_CO_NODE );
    EQASSERT( command->command == CMD_NODE_MAP_OBJECT );

    NodePtr node = command.getNode();
    NodeMapObjectPacket* packet =
        command.get<NodeMapObjectPacket>();
    const uint32_t instanceID = packet->instanceID;
    const uint128_t version = packet->requestedVersion;
    EQASSERT( version == VERSION_OLDEST || version == VERSION_FIRST ||
              version == VERSION_NONE );

    const bool useCache = packet->masterInstanceID == _object->getInstanceID();
    reply.version = VERSION_FIRST;

    if( useCache &&
        packet->minCachedVersion == VERSION_FIRST && 
        packet->maxCachedVersion == VERSION_FIRST )
    {
#ifdef EQ_INSTRUMENT_MULTICAST
        ++_hit;
#endif
        reply.useCache = true;
        return;
    }

#ifdef EQ_INSTRUMENT_MULTICAST
    ++_miss;
#endif

    if( version != VERSION_NONE ) // send current data
    {
        // send instance data
        ObjectInstanceDataOStream os( this );

        os.enableMap( VERSION_FIRST, node, instanceID );
        _object->getInstanceData( os );
        os.disable();
        EQASSERTINFO( os.hasSentData(),
                      "Static objects should always serialize data" );

        if( os.hasSentData( ))
            return;
    }

    // no data, send empty packet to set version
    ObjectInstancePacket instancePacket;
    instancePacket.type = PACKETTYPE_CO_OBJECT;
    instancePacket.command = CMD_OBJECT_INSTANCE;
    instancePacket.version = VERSION_FIRST;
    instancePacket.last = true;
    instancePacket.instanceID = instanceID;
    instancePacket.masterInstanceID = _object->getInstanceID();
    _object->send( node, instancePacket );
}

}
