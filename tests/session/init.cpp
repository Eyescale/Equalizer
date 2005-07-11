
#include <eq/net/server.h>
#include <eq/net/session.h>

#include <eq/net/global.h>
#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    cout << "Create new session... " << endl;
    Session* session = Session::create(NULL);
    sleep(1);
}

