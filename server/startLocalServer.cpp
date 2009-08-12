
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "server.h"

#include "global.h"
#include "loader.h"

#include <eq/net/node.h>
#include <eq/net/pairConnection.h>

using namespace eq::server;
using namespace eq::base;
using namespace std;

#define CONFIG "server{ config{ appNode{ pipe {                            \
    window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}}    \
    compound { channel \"channel\" wall { bottom_left  [ -.8 -.5 -1 ]      \
                                          bottom_right [  .8 -.5 -1 ]      \
                                          top_left     [ -.8  .5 -1 ] }}}}"

namespace
{
class ServerThread : public eq::base::Thread
{
public:
    ServerThread() {}
    virtual ~ServerThread() {}

    bool start( ServerPtr server )
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

            EQINFO << "Server thread done, ref count " 
                   << _server->getRefCount() - 1 << endl;
            EQASSERTINFO( _server->getRefCount() == 1, _server->getRefCount( ));

            _server = 0;
            Global::clear();

            return reinterpret_cast< void* >( static_cast< size_t >(
                ret ? EXIT_SUCCESS : EXIT_FAILURE ));
        }

private:
    ServerPtr _server;    
};

static ServerThread _serverThread;
}

EQSERVER_EXPORT eq::net::ConnectionPtr eqsStartLocalServer( 
    const std::string& file )
{
    if( _serverThread.isRunning( ))
    {
        EQWARN << "Only one app-local per process server allowed" << endl;
        return 0;
    }

    Loader    loader;
    ServerPtr server;

    if( !file.empty( ))
        server = loader.loadFile( file );
    if( !server )
        server = loader.parseServer( CONFIG );
    if( !server )
    {
        EQERROR << "Failed to load configuration" << endl;
        return 0;
    }

    if( !server->listen( ))
    {
        EQERROR << "Failed to setup server listener" << endl;
        return 0;
    }

    Loader::addOutputCompounds( server );
    Loader::addDestinationViews( server );
    Loader::addDefaultObserver( server );

    eq::net::ConnectionDescriptionPtr desc = new ConnectionDescription;
    desc->type = eq::net::CONNECTIONTYPE_PIPE;

    // Do not use RefPtr for easier handling
    eq::net::PairConnection* connection = new eq::net::PairConnection( 
        eq::net::Connection::create( desc ),
        eq::net::Connection::create( desc ));

    // Wrap in one RefPtr to do correct reference counting and avoid deletion
    eq::net::ConnectionPtr  conn = connection;

    if( !connection->connect( ))
    {
        EQERROR << "Failed to connect server connection" << endl;
        return 0;
    }

    eq::net::ConnectionPtr sibling = connection->getSibling();
    server->_addConnection( sibling );

    if( !_serverThread.start( server ))
    {
        EQERROR << "Failed to start server thread" << endl;
        return 0;
    }

    return conn;
}

EQSERVER_EXPORT void eqsJoinLocalServer()
{
    _serverThread.join();
}
