
#include <eq/net/server.h>

#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    cout << "Create session... " << endl;
    const int ret = Server::run( ":4242" );
    cout << "Server exit value " << ret << endl;
    return ret;
}

