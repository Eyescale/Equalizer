
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

#include "staticMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"
#include "objectInstanceDataOStream.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
StaticMasterCM::StaticMasterCM( Object* object )
        : _object( object )
{}

StaticMasterCM::~StaticMasterCM()
{}

uint32_t StaticMasterCM::addSlave( Command& command )
{
    EQASSERT( command->datatype == DATATYPE_EQNET_SESSION );
    EQASSERT( command->command == CMD_SESSION_SUBSCRIBE_OBJECT );

    NodePtr node = command.getNode();
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    const uint32_t instanceID = packet->instanceID;
    EQASSERT( packet->requestedVersion == VERSION_OLDEST );

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

    ObjectInstanceDataOStream os( _object );
    os.setInstanceID( instanceID );
    os.setNodeID( node->getNodeID( ));
    os.enable( node );

    _object->getInstanceData( os );
    os.disable();
#ifdef EQ_INSTRUMENT_MULTICAST
    ++_miss;
#endif

    return VERSION_INVALID; // no data was in cache
}

}
}
