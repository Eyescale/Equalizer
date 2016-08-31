
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2010, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "localServer.h"

#include "server.h"

#include "global.h"
#include "loader.h"

#include <co/node.h>

#define CONFIG "server{ config{ appNode{ pipe {                            \
    window { viewport [ .25 .25 .5 .5 ] channel { name \"channel\" }}}}    \
    compound { channel \"channel\" wall { }}}}"

namespace eq
{
namespace server
{
namespace
{
class ServerThread : public lunchbox::Thread
{
public:
    ServerThread() {}
    virtual ~ServerThread() {}

    bool start( eq::server::ServerPtr server )
        {
            LBASSERT( !_server );
            _server = server;
            return Thread::start();
        }

    eq::server::ServerPtr getServer()
        {
            return _server;
        }

protected:

    void run() final
    {
        _server->run();
        _server->close();
        _server->deleteConfigs();

        LBASSERTINFO( _server->getRefCount() == 1,
                      "Server thread done, still referenced by " <<
                      _server->getRefCount( ));
        _server = 0;
        eq::server::Global::clear();
    }

private:
    eq::server::ServerPtr _server;
};

static ServerThread _serverThread;
}

bool startLocalServer( const std::string& config )
{
    if( _serverThread.isRunning( ))
    {
        LBWARN << "Only one app-local per process server allowed" << std::endl;
        return false;
    }

    eq::server::Loader loader;
    eq::server::ServerPtr server;

    if( config.length() > 3 &&
        config.compare( config.size() - 4, 4, ".eqc" ) == 0 )
    {
        server = loader.loadFile( config );
    }
    else
    {
#ifdef EQUALIZER_USE_HWSD
        server = new eq::server::Server; // configured upon Server::chooseConfig
#else
        server = loader.parseServer( CONFIG );
#endif
    }

    if( !server )
    {
        LBERROR << "Failed to load configuration" << std::endl;
        return false;
    }

    eq::server::Loader::addOutputCompounds( server );
    eq::server::Loader::addDestinationViews( server );
    eq::server::Loader::addDefaultObserver( server );
    eq::server::Loader::convertTo11( server );
    eq::server::Loader::convertTo12( server );
    // TODO: ref count is 2 since config holds ServerPtr
    // LBASSERTINFO( server->getRefCount() == 1, server );

    if( !server->listen( ))
    {
        LBERROR << "Failed to setup server listener" << std::endl;
        return false;
    }

    if( !_serverThread.start( server ))
    {
        LBERROR << "Failed to start server thread" << std::endl;
        return false;
    }

    return true;
}

co::ConnectionPtr connectLocalServer()
{
    if( !_serverThread.getServer( ))
        return nullptr;

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_PIPE;
    co::ConnectionPtr connection = co::Connection::create( desc );
    if( !connection->connect( ))
    {
        LBERROR << "Failed to set up server connection" << std::endl;
        return nullptr;
    }

    co::ConnectionPtr sibling = connection->acceptSync();

    _serverThread.getServer()->addConnection( sibling );

    return connection;
}

void joinLocalServer()
{
    _serverThread.join();
}

}
}
