
#include "test.h"

#include <eq/client/nodeFactory.h>
#include <eq/net/nodeID.h>

#include <stdlib.h>

using namespace eqNet;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

int main( int argc, char **argv )
{
    NodeID id1( true );
    NodeID id2( true );

    TEST( id1 );
    TEST( id1 != id2 );

    id1 = id2;
    TEST( id1 == id2 );
    
    NodeID* id3 = new NodeID( id1 );
    NodeID* id4 = new NodeID( true );

    TEST( id1 == *id3 );
    TEST( *id4 != *id3 );
    
    *id4 = *id3;
    TEST( *id4 == *id3 );
    
    delete id3;
    delete id4;

    NodeID id5, id6;
    TEST( !id5 );
    TEST( id5 == id6 );
    
    return EXIT_SUCCESS;
}

