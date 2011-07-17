
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "slaveConfig.h"

#include "objectMap.h"

#include <sequel/application.h>

namespace seq
{
namespace detail
{

bool SlaveConfig::mapData( const uint128_t& initID )
{
    EQASSERT( !_objects );

    _objects = new ObjectMap( *getApplication( ));
    const uint32_t request = mapObjectNB( _objects, initID, co::VERSION_OLDEST,
                                          getApplicationNode( ));
    if( !mapObjectSync( request ))
        return false;
    return true;
}

void SlaveConfig::syncData( const uint128_t& version )
{
    EQASSERT( _objects )
    _objects->sync( version );
}

void SlaveConfig::unmapData()
{
    EQASSERT( _objects )
    unmapObject( _objects );
    delete _objects;
    _objects = 0;
}

}
}
