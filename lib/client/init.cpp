
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "client.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "version.h"

#include <eq/net/init.h>

using namespace eq;
using namespace eqBase;
using namespace std;

static NodeFactory* _allocatedNodeFactory = 0;

EQ_EXPORT bool eq::init( int argc, char** argv, NodeFactory* nodeFactory )
{
    EQINFO << "Equalizer v" << Version::getString() << " initializing" << endl;

    // We do not use getopt_long because of:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages

    for( int i=1; i<argc; ++i )
    {
        if( strcmp( "--eq-server", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
                Global::setServer( argv[i] );
        }
    }
    
	if( nodeFactory )
		Global::_nodeFactory = nodeFactory;
	else
	{
		_allocatedNodeFactory = new NodeFactory();
		Global::_nodeFactory = _allocatedNodeFactory;
	}

    RefPtr<eqNet::Node> node = new Client;
    eqNet::Node::setLocalNode( node );

    char** argvListen = static_cast<char**>( alloca( (argc+1)*sizeof( char* )));
    
    for( int i=0; i<argc; i++ )
        argvListen[i] = argv[i];

    argvListen[argc] = "--eq-listen";

    if( !eqNet::init( argc+1, argvListen ))
    {
        EQERROR << "Failed to initialise Equalizer network layer" << endl;
        eqNet::Node::setLocalNode( NULL );
        return false;
    }

    return true;
}

EQ_EXPORT bool eq::exit()
{
    return eqNet::exit();

    Global::_nodeFactory = 0;
	delete _allocatedNodeFactory;
	_allocatedNodeFactory = 0;
}
