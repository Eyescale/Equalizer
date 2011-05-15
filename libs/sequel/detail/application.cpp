
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

#include "application.h"
#include "../application.h"

#include "node.h"
#include "objectMap.h"

#include <eq/config.h>
#include <eq/configParams.h>
#include <eq/init.h>
#include <eq/server.h>

#include <eq/pipe.h>

namespace seq
{
namespace detail
{

Application::Application( ApplicationPtr app )
        : _app( app )
        , _config( 0 )
        , _objects( 0 )
{}

Application::~Application()
{
    EQASSERT( !_config );
    _app = 0;
}

bool Application::init()
{
    eq::ServerPtr server = new eq::Server;
    if( !_app->connectServer( server ))
    {
        EQERROR << "Can't open Equalizer server" << std::endl;
        exit();
        return false;
    }

    eq::ConfigParams cp;
    _config = static_cast< Config* >( server->chooseConfig( cp ));

    if( !_config )
    {
        EQERROR << "No matching configuration on Equalizer server" << std::endl;
        _app->disconnectServer( server );
        exit();
        return false;
    }

    _objects = new ObjectMap( _config );
    EQCHECK( _config->registerObject( _objects ));

    if( !_config->init( _objects->getID( )))
    {
        EQWARN << "Error during configuration initialization: "
               << _config->getError() << std::endl;
        exit();
        return false;
    }
    if( _config->getError( ))
        EQWARN << "Error during configuration initialization: "
               << _config->getError() << std::endl;
    return true;
}

bool Application::exit()
{
    bool retVal = true;
    if( _config )
    {
        eq::ServerPtr server = _config->getServer();

        if( !_config->exit( ))
            retVal = false;
        if( _objects )
            _config->deregisterObject( _objects );
        server->releaseConfig( _config );
        if( !_app->disconnectServer( server ))
            retVal = false;
    }

    delete _objects;
    _objects = 0;
    _config = 0;
    return retVal;
}

bool Application::mapData( const uint128_t& initID )
{
    EQASSERT( _config );
    if( !_config )
        return false;

    if( _objects )
    {
        // appNode, _initData is registered already
        EQASSERT( _objects->isMaster( ));
        EQASSERT( _objects->getID() == initID );
        return true;
    }

    _objects = new ObjectMap( _config );
    EQCHECK( _config->registerObject( _objects ));
    const uint32_t request =
        _config->mapObjectNB( _objects, initID, co::VERSION_OLDEST,
                              _config->getApplicationNode( ));
    if( !_config->mapObjectSync( request ))
        return false;
    return true;
}

void Application::unmapData()
{
    EQASSERT( _config && _objects );

    if( !_config || !_objects || _objects->isMaster( ))
        return;

    _config->unmapObject( _objects );
    delete _objects;
    _objects = 0;
}

eq::Node* Application::createNode( eq::Config* parent )
{
    return new Node( this, parent );
}

eq::Pipe* Application::createPipe( eq::Node* parent )
{
    return new Pipe( parent );
}

}
}
