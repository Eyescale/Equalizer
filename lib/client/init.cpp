
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "init.h"

#include "client.h"
#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "server.h"
#include "version.h"

#include <eq/net/init.h>

#ifdef EQ_USE_PARACOMP
#  include <pcapi.h>
#endif

using namespace std;

namespace eq
{

EQ_EXPORT bool init( const int argc, char** argv, NodeFactory* nodeFactory )
{
    EQINFO << "Equalizer v" << Version::getString() << " initializing" << endl;

#ifdef AGL
    ProcessSerialNumber selfProcess = { 0, kCurrentProcess };
    SetFrontProcess( &selfProcess );
    GetCurrentEventQueue();
#endif

#ifdef EQ_USE_PARACOMP
    EQINFO << "Initializing Paracomp library" << endl;
    PCerr err = pcSystemInitialize( 0 );
    if( err != PC_NO_ERROR )
    {
        EQERROR << "Paracomp initialization failed: " << err << endl;
        return false;
    }
#endif

    // We do not use getopt_long because of:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages

    for( int i=1; i<argc; ++i )
    {
        if( strcmp( "--eq-server", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
                Global::setServer( argv[i] );
        }
        else if( strcmp( "--eq-config", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
                Global::setConfigFile( argv[i] );
        }
    }
    
	EQASSERT( nodeFactory );
    Global::_nodeFactory = nodeFactory;

    return net::init( argc, argv );
}

EQ_EXPORT bool exit()
{
#ifdef EQ_USE_PARACOMP
    pcSystemFinalize();
#endif

    Global::_nodeFactory = 0;
    return net::exit();
}

EQ_EXPORT Config* getConfig( const int argc, char** argv )
{
    // 1. initialization of a local client node
    ClientPtr client = new Client;
    if( client->initLocal( argc, argv ))
    {
        // 2. connect to server
        ServerPtr server = new Server;
        if( client->connectServer( server ))
        {
            // 3. choose configuration
            ConfigParams configParams;
            Config* config = server->chooseConfig( configParams );
            if( config )
                return config;

            EQERROR << "No matching config on server" << endl;

            // -2. disconnect server
            client->disconnectServer( server );
        }
        else
            EQERROR << "Can't open server" << endl;
        
        // -1. exit local client node
        client->exitLocal();
    }
    else
        EQERROR << "Can't init local client node" << endl;

    return 0;
}

EQ_EXPORT void releaseConfig( Config* config )
{
    if( !config )
        return;

    ServerPtr server = config->getServer();
    EQASSERT( server.isValid( ));
    server->releaseConfig( config );

    ClientPtr client = server->getClient();
    EQASSERT( client.isValid( ));
    client->disconnectServer( server );

    client->exitLocal();

    EQASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    EQASSERTINFO( server->getRefCount() == 1, server->getRefCount( ));
}

}
