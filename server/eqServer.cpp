
#include "server.h"

#include "global.h"
#include "loader.h"

#include <eq/net/init.h>

#include <iostream>

using namespace eqs;
using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    Loader loader;
    RefPtr<Server> server = loader.loadFile( argc > 1 ? argv[1] :
                                             "examples/configs/config.eqc" );
    if( !server.isValid( ))
    {
        EQERROR << "Server load failed" << endl;
        return EXIT_FAILURE;
    }

    eqNet::Global::setDefaultPort( EQ_DEFAULT_PORT );
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

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
    return EXIT_SUCCESS;
}

