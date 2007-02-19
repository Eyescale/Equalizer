
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "global.h"
#include "node.h"
#include "socketConnection.h"

#ifdef WIN32
#  include <direct.h>
#  define getcwd _getcwd
#endif

using namespace eqNet;
using namespace eqBase;
using namespace std;

static bool initLocalNode( int argc, char** argv );
static bool exitLocalNode();

EQ_EXPORT bool eqNet::init( int argc, char** argv )
{
    EQINFO << "Log level " << Log::getLogLevelString() << " topics " 
           << Log::topics << endl;

    EQASSERT( argc > 0 );

#ifdef WIN32
    WORD    wsVersion = MAKEWORD( 2, 0 );
    WSADATA wsData;
    if( WSAStartup( wsVersion, &wsData ) != 0 )
    {
        EQERROR << "Initialization of Windows Sockets failed" 
                << getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif

    const string programName = Global::getProgramName();
    if( programName.size() == 0  )
        Global::setProgramName( argv[0] );

    const string workDir = Global::getWorkDir();
    if( workDir.size() == 0 )
    {
        char cwd[MAXPATHLEN];
        getcwd( cwd, MAXPATHLEN );

        Global::setWorkDir( cwd );
    }

    if( !initLocalNode( argc, argv ))
    {
        EQERROR << "Could not initialize local node" << endl;
        return false;
    }
    return true;
}

bool initLocalNode( int argc, char** argv )
{
    EQINFO << "args: ";
    for( int i=0; i<argc; i++ )
         EQINFO << argv[i] << ", ";
    EQINFO << endl;

    // We do not use getopt_long because it really does not work due to the
    // following aspects:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages
    bool   listen   = false;
    bool   isClient = false;
    string listenOpts;
    string clientOpts;

    for( int i=1; i<argc; ++i )
    {
        if( strcmp( "--eq-listen", argv[i] ) == 0 )
        {
            listen = true;
            ++i;
            if( i<argc && argv[i][0] != '-' )
                listenOpts = argv[i];
            else
                --i;
        }
        else if( strcmp( "--eq-client", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc && argv[i][0] != '-' )
            {
                isClient   = true;
                clientOpts = argv[i];
            }
        }
    }

    if( listen )
    {
        EQINFO << "Listener port requested" << endl;
        RefPtr<Connection>            connection = new SocketConnection();
        RefPtr<ConnectionDescription> connDesc   = connection->getDescription();

        if( !listenOpts.empty() && !connDesc->fromString( listenOpts ))
            EQINFO << "No listening port parameters read from command line"
                   << endl;
        EQINFO << "Listening connection description: " << connDesc->toString()
               << endl;

        if( !connection->listen( ))
        {
            EQWARN << "Can't create listening connection" << endl; 
            return false;
        }

        RefPtr<Node> localNode = Node::getLocalNode();
        if( !localNode.isValid( ))
        {
            localNode = new Node();
            Node::setLocalNode( localNode );
        }

        if( !localNode->listen( connection ))
        {
            EQWARN << "Can't create listener node" << endl; 
            return false;
        }
    }

    if( isClient )
    {
        EQINFO << "Client node started from command line with option " 
             << clientOpts << endl;
        RefPtr<Node> localNode = Node::getLocalNode();
        EQASSERT( localNode );

        return localNode->runClient( clientOpts );
    }

    return true;
}

EQ_EXPORT bool eqNet::exit()
{
    if( !exitLocalNode( ))
        return false;

#ifdef WIN32
    if( WSACleanup() != 0 )
    {
        EQERROR << "Cleanup of Windows Sockets failed" 
                << getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif

    return true;
}

bool exitLocalNode()
{
    RefPtr<Node> localNode = Node::getLocalNode();
    if( !localNode->stopListening( ))
        return false;

    return true;
}
