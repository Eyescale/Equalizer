
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

#include "channel.h"
#include "masterConfig.h"
#include "node.h"
#include "pipe.h"
#include "slaveConfig.h"

#include <eq/sequel/application.h>
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
        , _isMaster( false )
{}

Application::~Application()
{
    EQASSERT( !_config );
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

bool Application::init( co::Object* initData )
{
    _isMaster = true;
    eq::ServerPtr server = new eq::Server;
    if( !_app->connectServer( server ))
    {
        EQERROR << "Can't open Equalizer server" << std::endl;
        exit();
        return false;
    }

    const eq::ConfigParams cp;
    _config = static_cast< Config* >( server->chooseConfig( cp ));

    if( !_config )
    {
        EQERROR << "No matching configuration on Equalizer server" << std::endl;
        _app->disconnectServer( server );
        exit();
        return false;
    }

    if( !_config->init( initData ))
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

    _config = 0;
    _isMaster = false;
    return retVal;
}

bool Application::run()
{
    while( _config->isRunning( ))
    {
        _config->startFrame();
        if( _config->getError( ))
            EQWARN << "Error during frame start: " << _config->getError()
                   << std::endl;
        _config->finishFrame();

        while( !_config->needRedraw( )) // wait for an event requiring redraw
        {
            if( _app->hasCommands( )) // execute non-critical pending commands
            {
                _app->processCommand();
                _config->handleEvents(); // non-blocking
            }
            else  // no pending commands, block on user event
            {
                const eq::ConfigEvent* event = _config->nextEvent();
                if( !_config->handleEvent( event ))
                    EQVERB << "Unhandled " << event << std::endl;
            }
        }
        _config->handleEvents(); // process all pending events
    }
    _config->finishAllFrames();
    return true;
}

eq::Config* Application::createConfig( eq::ServerPtr parent )
{
    if( isMaster( ))
        return new MasterConfig( parent );
    return new SlaveConfig( parent );
}

eq::Node* Application::createNode( eq::Config* parent )
{
    return new Node( parent );
}

eq::Pipe* Application::createPipe( eq::Node* parent )
{
    return new Pipe( parent );
}

eq::Channel* Application::createChannel( eq::Window* parent )
{
    return new Channel( parent );
}

}
}
