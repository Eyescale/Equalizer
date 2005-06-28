
#include <eq/net/session.h>

#include <eq/net/global.h>
#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    cout << "Create session... " << endl;
    Session* session = Session::create(NULL);
    if( session )
        cout << "Got session " << session->getID() << endl;
    else
        cout << "Got no session" << endl;
    sleep(1);
}

