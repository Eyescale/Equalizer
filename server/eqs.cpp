
#include <eq/server/server.h>

#include <iostream>

using namespace eqs;
using namespace std;

int main( int argc, char **argv )
{
    Server server;

    const bool result = server.run( argc, argv );
    if( !result )
        ERROR << "Server did not run correctly. Please consult log." << endl;
}

