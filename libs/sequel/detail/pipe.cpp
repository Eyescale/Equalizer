
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

#include "pipe.h"

#include "config.h"
#include "node.h"
#include "objectMap.h"

#include <eq/sequel/application.h>
#include <eq/sequel/error.h>
#include <eq/sequel/renderer.h>

namespace seq
{
namespace detail
{

Pipe::Pipe( eq::Node* parent )
        : eq::Pipe( parent ) 
        , _objects( 0 )
        , _renderer( 0 )
{}

Pipe::~Pipe()
{
    EQASSERT( !_objects );
}

seq::Application* Pipe::getApplication()
{
    return getConfig()->getApplication();
}

Config* Pipe::getConfig()
{
    return getNode()->getConfig();
}

Node* Pipe::getNode()
{
    return static_cast< Node* >( eq::Pipe::getNode( ));
}

bool Pipe::configInit( const uint128_t& initID )
{
    if( !eq::Pipe::configInit( initID ))
        return false;

    EQASSERT( !_renderer );
    _renderer = getApplication()->createRenderer();
    if( !_renderer )
    {
        EQASSERT( _renderer );
        setError( ERROR_SEQUEL_CREATERENDERER_FAILED );
        return false;
    }

    if( _mapData( initID ))
        return true;

    setError( ERROR_SEQUEL_MAPOBJECT_FAILED );
    return false;
}

bool Pipe::configExit()
{
    _unmapData();

    if( _renderer )
        getApplication()->destroyRenderer( _renderer );
    _renderer = 0;

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint128_t& frameID, const uint32_t frameNumber )
{
    _syncData( frameID );
    return eq::Pipe::frameStart( frameID, frameNumber );
}

bool Pipe::_mapData( const uint128_t& initID )
{
    EQASSERT( !_objects );
    EQASSERT( _renderer );

    _objects = new ObjectMap( *_renderer );
    Config* config = getConfig();
    const uint32_t request = config->mapObjectNB( _objects, initID,
                                                  co::VERSION_OLDEST,
                                                  config->getApplicationNode());
    if( !config->mapObjectSync( request ))
        return false;
    return true;
}

void Pipe::_syncData( const uint128_t& version )
{
    EQASSERT( _objects )
    _objects->sync( version );
}

void Pipe::_unmapData()
{
    EQASSERT( _objects )
        getConfig()->unmapObject( _objects );
    delete _objects;
    _objects = 0;
}

}
}
