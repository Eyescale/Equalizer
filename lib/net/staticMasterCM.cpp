
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "object.h"
#include "packets.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
StaticMasterCM::StaticMasterCM( Object* object ) 
        : _object( object )
        , _os( this )
{}

StaticMasterCM::~StaticMasterCM()
{}

uint32_t StaticMasterCM::addSlave( Command& command )
{
    EQASSERT( command->type == PACKETTYPE_EQNET_SESSION );
    EQASSERT( command->command == CMD_SESSION_SUBSCRIBE_OBJECT );

    NodePtr node = command.getNode();
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
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
}
