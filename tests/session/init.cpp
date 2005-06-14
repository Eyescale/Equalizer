
#include <eq/net/session.h>

#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    cout << "Created session " << Session::create(NULL);
}

