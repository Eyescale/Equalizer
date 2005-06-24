
#include <eq/net/session.h>

#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    char dummy;
    //cin >> dummy;
    cout << "Create session... " << endl;
    const uint sessionID = Session::create(NULL);
    cout << "Got session " << sessionID << endl;
    //cin >> dummy;
}

