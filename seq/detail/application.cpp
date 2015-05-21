
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "channel.h"
#include "masterConfig.h"
#include "node.h"
#include "pipe.h"
#include "slaveConfig.h"
#include "view.h"
#include "window.h"

#include <seq/application.h>
#include <eq/config.h>
#include <eq/init.h>
#include <eq/pipe.h>
#include <eq/server.h>
#include <eq/fabric/configParams.h>

namespace seq
{
namespace detail
{

Application::Application( ApplicationPtr app, co::Object* initData )
        : _app( app )
        , _config( 0 )
        , _initData( initData )
        , _isMaster( false )
{}

Application::~Application()
{
    LBASSERT( !_config );
    _app = 0;
}

Config* Application::getConfig()
{
    return _config;
}

const Config* Application::getConfig() const
{
    return _config;
}

bool Application::init()
{
    _isMaster = true;
    eq::ServerPtr server = new eq::Server;
    if( !_app->connectServer( server ))
    {
        LBERROR << "Can't open Equalizer server" << std::endl;
        exit();
        return false;
    }

    const eq::fabric::ConfigParams cp;
    _config = static_cast< Config* >( server->chooseConfig( cp ));

    if( !_config )
    {
        LBERROR << "No matching configuration on Equalizer server" << std::endl;
        _app->disconnectServer( server );
        exit();
        return false;
    }

    if( !_config->init( ))
        return false;
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

        server->releaseConfig( _config );
        if( !_app->disconnectServer( server ))
            retVal = false;
    }

    LBASSERT( !_config );
    _isMaster = false;
    return retVal;
}

bool Application::run( co::Object* frameData )
{
    return _config->run( frameData );
}

eq::Config* Application::createConfig( eq::ServerPtr parent )
{
    if( isMaster( ))
        return new MasterConfig( parent );

    LBASSERT( !_config );
    _config = new SlaveConfig( parent );
    return _config;
}

void Application::releaseConfig( eq::Config* config )
{
    LBASSERT( config == _config );
    _config = 0;
    delete config;
}

eq::View* Application::createView( eq::Layout* parent )
{
    return new View( parent );
}

eq::Node* Application::createNode( eq::Config* parent )
{
    return new Node( parent );
}

eq::Pipe* Application::createPipe( eq::Node* parent )
{
    return new Pipe( parent );
}

eq::Window* Application::createWindow( eq::Pipe* parent )
{
    return new Window( parent );
}

eq::Channel* Application::createChannel( eq::Window* parent )
{
    return new Channel( parent );
}

}
}
