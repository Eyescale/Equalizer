
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

#include "objectCM.h"

#include "command.h"
#include "nullCM.h"
#include "node.h"
#include "nodePackets.h"
#include "object.h"
#include "objectInstanceDataOStream.h"
#include "objectPackets.h"

co::ObjectCM* co::ObjectCM::ZERO = new co::NullCM;

#ifdef EQ_INSTRUMENT_MULTICAST
lunchbox::a_int32_t co::ObjectCM::_hit( 0 );
lunchbox::a_int32_t co::ObjectCM::_miss( 0 );
#endif

namespace co
{
ObjectCM::ObjectCM( Object* object )
        : _object( object )
{}

void ObjectCM::push( const uint128_t& groupID, const uint128_t& typeID,
                     const Nodes& nodes )
{
    EQASSERT( _object );
    EQASSERT( !nodes.empty( ));
    if( nodes.empty( ))
        return;

    ObjectInstanceDataOStream os( this );
    os.enablePush( getVersion(), nodes );
    _object->getInstanceData( os );

    NodeObjectPushPacket pushPacket( _object->getID(), groupID, typeID );
    os.disable( pushPacket );
}

}
