
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

#include "node.h"

#include "application.h"
#include "config.h"

#include <sequel/application.h>
#include <sequel/error.h>

namespace seq
{
namespace detail
{

Node::Node( eq::Config* parent )
        : eq::Node( parent ) 
{}

Config* Node::getConfig()
{
    return static_cast< Config* >( eq::Node::getConfig( ));
}

seq::Application* Node::getApplication()
{
    return getConfig()->getApplication();
}

bool Node::configInit( const uint128_t& initID )
{
    if( !eq::Node::configInit( initID ))
        return false;

    Config* config = getConfig();
    if( !config->mapData( initID ))
    {
        setError( ERROR_SEQUEL_MAPOBJECT_FAILED );
        return false;
    }

    co::Object* initData = config->getInitData();
    getApplication()->clientInit( initData );
    return true;
}

bool Node::configExit()
{
    getApplication()->clientExit();
    getConfig()->unmapData();
    return eq::Node::configExit();
}

void Node::frameStart( const uint128_t& frameID, const uint32_t frameNumber )
{
    getConfig()->syncData( frameID );
    return eq::Node::frameStart( frameID, frameNumber );
}

}
}
