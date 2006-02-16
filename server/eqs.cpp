
#include "server.h"

#include <iostream>

using namespace eqs;
using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    RefPtr<Server> server = new Server;

    const bool result = server->run( argc, argv );
    if( !result )
        EQERROR << "Server did not run correctly. Please consult log." << endl;

    EQINFO << "Server ref count: " << server->getRefCount() << endl;
}

