
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

    const string programName = Global::getProgramName();
    if( programName.size() == 0  )
    {
        if( argv[0][0] == '/' )
            Global::setProgramName( argv[0] );
        else
        {
            const string pwd = getenv("PWD");
            Global::setProgramName( pwd + "/" + argv[0] );
        }
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
    for( int i=0; i<argc; i++ )
        INFO << "arg " << argv[i] << endl;

    struct option options[] = {
        { "eq-listen",      no_argument,       NULL, 'l' },
        { "eq-client",      required_argument, NULL, 'c' },
        { NULL,             0,                 NULL,  0 }
    };

    bool   listen   = false;
    bool   isClient = false;
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
                isClient   = true;
                clientOpts = optarg;
                break;

            default:
                WARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }
    
    if( listen )
    {
        INFO << "Listener port requested" << endl;
        // TODO: connection description parameters from argv
        RefPtr<Connection> connection = Connection::create( eqNet::TYPE_TCPIP );
        RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;

        if( !connection->listen( connDesc ))
        {
            WARN << "Can't create listening connection" << endl; 
            if( isClient )
                exit( EXIT_FAILURE );
            return false;
        }

        Node* localNode = Node::getLocalNode();
        if( localNode == NULL )
        {
            localNode = new Node();
            Node::setLocalNode( localNode );
        }

        if( !localNode->listen( connection ))
        {
            WARN << "Can't create listener node" << endl; 
            if( isClient )
                exit( EXIT_FAILURE );
            return false;
        }
    }

    INFO << "clientOpts: " << clientOpts << endl;
    if( isClient )
    {
        INFO << "Client node started from command line with option " 
             << clientOpts << endl;
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
