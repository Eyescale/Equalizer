
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "global.h"
#include "node.h"

#include <getopt.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

static bool initLocalNode( int argc, char** argv );
static bool exitLocalNode();

bool eqNet::init( int argc, char** argv )
{
    ASSERT( argc > 0 );

    if( Global::getProgramName().size() == 0 )
    {
        const string pwd = getenv("PWD");
        Global::setProgramName( pwd + "/" + argv[0] );
    }

    if( !initLocalNode( argc, argv ))
    {
        ERROR << "Could not initialize local node" << endl;
        return false;
    }

    return true;
}

bool initLocalNode( int argc, char** argv )
{
    struct option options[] = {
        { "eq-listen",      no_argument,       NULL, 'l' },
        { "eq-client",      required_argument, NULL, 'c' },
        { NULL,             0,                 NULL,  0 }
    };

    bool   listen  = false;
    string clientOpts;
    int    result;
    int    index;

    while( (result = getopt_long( argc, argv, "", options, &index )) != -1 )
    {
        switch( result )
        {
            case 'l':
                listen = true;
                break;

            case 'c':
                clientOpts = optarg;
                break;

            default:
                WARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }
    
    if( listen )
    {
        // TODO: connection description parameters from argv
        RefPtr<Connection> connection = Connection::create( eqNet::TYPE_TCPIP );
        RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;

        if( !connection->listen( connDesc ))
            return false;
        
        Node* localNode = Node::getLocalNode();
        if( localNode == NULL )
        {
            localNode = new Node();
            Node::setLocalNode( localNode );
        }

        if( !localNode->listen( connection ))
            return false;
    }

    if( clientOpts.size() > 0 )
    {
        Node* localNode = Node::getLocalNode();
        ASSERT( localNode );

        const bool ret = localNode->runClient( clientOpts );
        exit( ret ? EXIT_SUCCESS : EXIT_FAILURE );
    }

    return true;
}

bool eqNet::exit()
{
    if( !exitLocalNode( ))
        return false;

    return true;
}

bool exitLocalNode()
{
    Node* localNode = Node::getLocalNode();
    if( !localNode->stopListening( ))
        return false;

    return true;
}
