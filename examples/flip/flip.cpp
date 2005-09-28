
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <eq/eq.h>

#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }

int main( int argc, char** argv )
{
    if( !eq::init( argc, argv ))
        abort();

    eq::Server server;
    string address = "localhost:4242";
    if( !server.open( address ))
        DIE("Can't open server.");

    eq::ConfigParams params;
    params.appName = "foo";

    eq::Config* config = server.chooseConfig( &params );
    if( !config )
        DIE("No matching config on server.");

    //config->setWindowInitCB(...);

    if( !config->init( ))
        DIE("Config initialisation failed.");
    
//     while( running )
//     {
//         // update database

//         config->frameBegin( void* frameData );
//         config->renderData(...);
//         ...;
//         config->frameEnd();

//         // process events
//     }

    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    return EXIT_SUCCESS;
}

