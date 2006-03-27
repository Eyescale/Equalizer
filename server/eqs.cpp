
#include "server.h"

#include "loader.h"

#include <iostream>

using namespace eqs;
using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
#ifdef EQLOADER
    Loader loader;
    RefPtr<Server> server = loader.loadConfig( "examples/configs/config.eqc" );
#else
    RefPtr<Server> server = new Server;
#endif
    
    if( !server.isValid( ))
    {
        EQERROR << "Server load failed" << endl;
        return EXIT_FAILURE;
    }

    const bool result = server->run( argc, argv );
    if( !result )
        EQERROR << "Server did not run correctly. Please consult log." << endl;

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
    return EXIT_SUCCESS;
}

