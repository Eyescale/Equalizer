
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


namespace co
{
StaticMasterCM::StaticMasterCM( Object* object ) 
        : _object( object )
#pragma warning(push)
#pragma warning(disable: 4355)
        , _os( this )
#pragma warning(pop)
{}

StaticMasterCM::~StaticMasterCM(){}

uint128_t StaticMasterCM::addSlave( Command& command )
{
    EQASSERT( command->type == PACKETTYPE_CO_NODE );
    EQASSERT( command->command == CMD_NODE_MAP_OBJECT );

    NodePtr node = command.getNode();
    NodeMapObjectPacket* packet =
        command.getPacket<NodeMapObjectPacket>();
    const uint32_t instanceID = packet->instanceID;
    EQASSERT( packet->requestedVersion == VERSION_OLDEST ||
              packet->requestedVersion == VERSION_NONE );

    const bool useCache = packet->masterInstanceID == _object->getInstanceID();

    if( useCache &&
        packet->minCachedVersion == VERSION_NONE && 
        packet->maxCachedVersion == VERSION_NONE )
    {
#ifdef EQ_INSTRUMENT_MULTICAST
        ++_hit;
#endif
        return VERSION_NONE;
    }

    _os.reset();
    _os.setInstanceID( instanceID );
    _os.setNodeID( node->getNodeID( ));
    _os.enable( node, true );

    _object->getInstanceData( _os );

    _os.disable();
#ifdef EQ_INSTRUMENT_MULTICAST
    ++_miss;
#endif

    return VERSION_INVALID; // no data was in cache
}

}
