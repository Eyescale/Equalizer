
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "loader.h"

#include <eq/net/node.h>
#include <eq/net/pairConnection.h>
#include <eq/net/pipeConnection.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

#define CONFIG "server{ config{ appNode{ pipe { window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}} compound { channel \"channel\" wall { bottom_left [ -.8 -.5 -1 ] bottom_right [  .8 -.5 -1 ] top_left [ -.8  .5 -1 ] }}}}"

namespace
{
class ServerThread : public eqBase::Thread
{
public:
    ServerThread() {}
    virtual ~ServerThread() {}

    bool start( RefPtr<Server> server )
        {
            EQASSERT( !_server );
            _server = server;
            return Thread::start();
        }
    
protected:

    virtual void* run()
        {
            const bool ret = _server->run();
            EQASSERT( ret );

            _server->stopListening();

            // TODO EQASSERTINFO( _server->getRefCount() == 1, _server->getRefCount( ));
            _server = 0;
            return 0;
        }

private:
    RefPtr<Server> _server;    
};

static ServerThread _serverThread;
}

EQS_EXPORT eqBase::RefPtr< eqNet::Connection > eqsStartLocalServer()
{
    if( _serverThread.isRunning( ))
    {
        EQWARN << "Only one app-local per process server allowed" << endl;
        return 0;
    }

    Loader loader;
    RefPtr<Server> server = loader.parseServer( CONFIG );
    EQASSERT( server.isValid( ));

    if( !server->listen( ))
    {
        EQERROR << "Failed to setup server listener" << endl;
        return 0;
    }

    // Do not use RefPtr for easier handling
    eqNet::PairConnection* connection = new eqNet::PairConnection( 
        eqNet::Connection::create( eqNet::CONNECTIONTYPE_PIPE ),
        eqNet::Connection::create( eqNet::CONNECTIONTYPE_PIPE ));

    // Wrap in one RefPtr to do correct reference counting and avoid deletion
    eqNet::ConnectionPtr  conn = connection;

    if( !connection->connect( ))
    {
        EQERROR << "Failed to connect server connection" << endl;
        return 0;
    }

    server->_connectionSet.addConnection( connection->getSibling( ));
    if( !_serverThread.start( server ))
    {
        EQERROR << "Failed to start server thread" << endl;
        return 0;
    }

    return conn;
}

EQS_EXPORT void eqsJoinLocalServer()
{
    _serverThread.join();
}
