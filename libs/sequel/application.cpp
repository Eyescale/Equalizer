
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

#include <eq/config.h>
#include <eq/configParams.h>
#include <eq/init.h>
#include <eq/server.h>

namespace seq
{

Application::Application()
        : _config( 0 )
{}

Application::~Application()
{
    EQASSERT( !_config );
}

bool Application::init( const int argc, char** argv )
{
    if( _config )
    {
        EQERROR << "Already initialized" << std::endl;
        return false;
    }

    if( !eq::init( argc, argv, this ))
    {
        EQERROR << "Equalizer initialization failed" << std::endl;
        return false;
    }

    if( !initLocal( argc, argv ))
    {
        EQERROR << "Can't initialization client node" << std::endl;
        eq::exit();
        return false;
    }

    eq::ServerPtr server = new eq::Server;
    if( !connectServer( server ))
    {
        EQERROR << "Can't open Equalizer server" << std::endl;
        return EXIT_FAILURE;
    }

    eq::ConfigParams configParams;
    eq::Config* config = server->chooseConfig( configParams );

    if( !config )
    {
        EQERROR << "No matching configuration on Equalizer server" << std::endl;
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    if( !config->init( 0 ))
    {
        EQWARN << "Error during configuration initialization: "
               << config->getError() << std::endl;
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }
    if( config->getError( ))
        EQWARN << "Error during configuration initialization: "
               << config->getError() << std::endl;

    _config = config;
    return true;
}

bool Application::run()
{
    return true;
}

bool Application::exit()
{
    if( !_config )
        return true;

    eq::ServerPtr server = _config->getServer();
    bool retVal = _config->exit();
    server->releaseConfig( _config );

    if( !disconnectServer( server ))
        retVal = false;

    if( !exitLocal( ))
        retVal = false;

    EQASSERTINFO( getRefCount() == 1, *this );
    if( !eq::exit( ))
        retVal = false;

    _config = 0;
    return retVal;
}

}
