
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"

#include <stdlib.h>

using namespace std;
using namespace eqBase;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig()  { return new ::Config; }
    virtual eq::Node*    createNode()    { return new ::Node; }
    virtual eq::Pipe*    createPipe()    { return new ::Pipe; }
    virtual eq::Channel* createChannel() { return new ::Channel; }
};

int main( int argc, char** argv )
{
    // 1. parse arguments
    LocalInitData initData;
    initData.parseArguments( argc, argv );

    // 2. initialisation of local client node
	NodeFactory nodeFactory;
	if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }

    RefPtr<EqPly> client = new EqPly( initData );
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 3. run client
    const int ret = client->run();

    // 4. cleanup and exit
    client->exitLocal();
    client = 0;

    eq::exit();
    return ret;
}
