
#include "server.h"

#include "global.h"
#include "loader.h"

#include <eq/net/init.h>

#include <iostream>

using namespace eqs;
using namespace eq::base;
using namespace std;

#define CONFIG "server{ config{ appNode{ pipe { window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}} compound { channel \"channel\" wall { bottom_left [ -.8 -.5 -1 ] bottom_right [  .8 -.5 -1 ] top_left [ -.8  .5 -1 ] }}}}"

int main( const int argc, char** argv )
{
    eq::net::init( argc, argv );
    eq::net::Global::setDefaultPort( EQ_DEFAULT_PORT );

    Loader loader;
    RefPtr<Server> server;

    if( argc == 1 )
    {
        server = loader.parseServer( CONFIG );
    }
    else
    {
        server = loader.loadFile( argv[1] );
    }

    if( !server.isValid( ))
    {
        EQERROR << "Server load failed" << endl;
        return EXIT_FAILURE;
    }

    if( !server->initLocal( argc, argv ))
    {
        EQERROR << "Can't create listener for server, please consult log" 
                << endl;
        return EXIT_FAILURE;
    }

    if( !server->run( ))
    {
        EQERROR << "Server did not run correctly, please consult log" << endl;
        return EXIT_FAILURE;
    }

    server->exitLocal();

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
    return EXIT_SUCCESS;
}

