
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#include "osgScaleViewer.h"

#include "config.h"

using namespace std;

OSGScaleViewer::OSGScaleViewer( const InitData& initData )
    : _initData( initData )
{
}

int OSGScaleViewer::run()
{
    // 1. connect to server
    eq::ServerPtr server = new eq::Server();
    if( !connectServer( server ))
    {
        std::cout << "Can't open server" << endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::ConfigParams configParams;
    Config* config = static_cast<Config*>( server->chooseConfig( configParams ));

    if( !config )
    {
        std::cout << "No matching config on server" << endl;
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    config->setInitData( _initData );
    if( !config->init( ))
    {
        std::cout << "Config initialization failed: "
                  << config->getErrorMessage() << endl;
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }

    // 4. run main loop
    while( config->isRunning( ))
    {
        config->startFrame();
        config->finishFrame();
    }
    config->finishAllFrames();

    // 5. exit config
    config->exit();

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        std::cout << "Client::disconnectServer failed" << endl;

    server = 0;

    return EXIT_SUCCESS;
}
