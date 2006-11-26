
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
    RefPtr<Server> server = loader.loadConfig( argc > 1 ? argv[1] :
                                               "examples/configs/config.eqc" );

    if( !server.isValid( ))
    {
        EQERROR << "Server load failed" << endl;
        return EXIT_FAILURE;
    }

    const bool result = server->run();
    if( !result )
        EQERROR << "Server did not run correctly, please consult log." << endl;

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
    return EXIT_SUCCESS;
}

