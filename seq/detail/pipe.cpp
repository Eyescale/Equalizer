
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
 *                          Petros Kataras <petroskataras@gmail.com>
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
#include "renderer.h"

#include <seq/application.h>
#include <seq/error.h>
#include <seq/renderer.h>

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
    LBASSERT( !_objects );
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

detail::Renderer* Pipe::getRendererImpl()
{
    LBASSERT( _renderer );
    if( !_renderer )
        return 0;
    return _renderer->getImpl();
}

co::Object* Pipe::getFrameData()
{
    LBASSERT( _objects );
    if( _objects )
        return _objects->getFrameData();
    return 0;
}

ObjectMap* Pipe::getObjectMap()
{
   return _objects;
}

bool Pipe::configInit( const uint128_t& initID )
{
    if( !eq::Pipe::configInit( initID ))
        return false;

    LBASSERT( !_renderer );
    _renderer = getApplication()->createRenderer();
    if( !_renderer )
    {
        LBASSERT( _renderer );
        sendError( ERROR_SEQUEL_CREATERENDERER_FAILED );
        return false;
    }
    getRendererImpl()->setPipe( this );

    if( _mapData( initID ))
        return true;

    sendError( ERROR_SEQUEL_MAPOBJECT_FAILED );
    return false;
}

bool Pipe::configExit()
{
    _unmapData();

    if( _renderer )
    {
        getRendererImpl()->setPipe( 0 );
        getApplication()->destroyRenderer( _renderer );
    }
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
    LBASSERT( !_objects );
    LBASSERT( _renderer );

    Config* config = getConfig();
    _objects = new ObjectMap( *config, *_renderer );
    const uint32_t request = config->mapObjectNB( _objects, initID,
                                                  co::VERSION_OLDEST,
                                                  config->getApplicationNode());
    if( !config->mapObjectSync( request ))
        return false;
    return true;
}

void Pipe::_syncData( const uint128_t& version )
{
    LBASSERT( _objects )
    _objects->sync( version );
}

void Pipe::_unmapData()
{
    LBASSERT( _objects )
        getConfig()->unmapObject( _objects );
    _objects->clear();
    delete _objects;
    _objects = 0;
}

}
}
