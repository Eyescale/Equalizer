
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "client.h"

#include "commandQueue.h"
#include "config.h"
#include "node.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "server.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/configVisitor.h>
#include <eq/fabric/leafVisitor.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/nodeType.h>
#include <eq/fabric/view.h>

#include <eq/server/localServer.h>

#include <co/iCommand.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/global.h>
#include <lunchbox/dso.h>
#include <lunchbox/file.h>
#include <boost/filesystem/path.hpp>

#ifdef EQ_QT_USED
#  include <QApplication> // must be included before any header defining Bool

#  include "os.h"
#else
class QApplication;
#endif

namespace eq
{
namespace
{
typedef stde::hash_set< Server* > ServerSet;
}

/** @cond IGNORE */
namespace detail
{
class Client
{
public:
    Client()
        : queue( co::Global::getCommandQueueLimit( ))
        , modelUnit( EQ_UNDEFINED_UNIT )
        , qtApp( 0 )
        , running( false )
    {}

    CommandQueue queue; //!< The command->node command queue.
    std::string name;
    Strings activeLayouts;
    ServerSet localServers;
    std::string gpuFilter;
    float modelUnit;
    QApplication* qtApp;
    bool running;

    void initQt( int argc LB_UNUSED, char** argv LB_UNUSED )
    {
#if EQ_GLX_USED || EQ_WGL_USED || EQ_AGL_USED
        return;
#endif
#ifdef EQ_QT_USED
        if( !QApplication::instance( ))
        {
#  ifdef __linux__
            ::XInitThreads();
#  endif
            qtApp = new QApplication( argc, argv );
        }
#endif
    }
};
}

typedef co::CommandFunc<Client> ClientFunc;

typedef fabric::Client Super;
/** @endcond */

Client::Client()
        : Super()
        , _impl( new detail::Client )
{
    registerCommand( fabric::CMD_CLIENT_EXIT,
                     ClientFunc( this, &Client::_cmdExit ), &_impl->queue );
    registerCommand( fabric::CMD_CLIENT_INTERRUPT,
                     ClientFunc( this, &Client::_cmdInterrupt ), &_impl->queue);

    LBVERB << "New client at " << (void*)this << std::endl;
}

Client::~Client()
{
    LBVERB << "Delete client at " << (void*)this << std::endl;
    LBASSERT( isClosed( ));
    close();
    delete _impl;
}

bool Client::connectServer( ServerPtr server )
{
    if( Super::connectServer( server ))
    {
        server->setClient( this );
        return true;
    }

    if( !server->getConnectionDescriptions().empty() ||
        !Global::getServer().empty() || getenv( "EQ_SERVER" ))
    {
        return false;
    }

    // Use app-local server if no explicit server was set
    if( !server::startLocalServer( Global::getConfig( )))
        return false;

    co::ConnectionPtr connection = server::connectLocalServer();
    if( !connection || !connect( server, connection ))
        return false;

    server->setClient( this );
    _impl->localServers.insert( server.get( ));
    return true;
}

bool Client::disconnectServer( ServerPtr server )
{
    ServerSet::iterator i = _impl->localServers.find( server.get( ));
    if( i == _impl->localServers.end( ))
    {
        server->setClient( 0 );
        const bool success = Super::disconnectServer( server );
        _impl->queue.flush();
        return success;
    }

    // shut down process-local server (see _startLocalServer)
    LBASSERT( server->isConnected( ));
    const bool success = server->shutdown();
    server::joinLocalServer();
    server->setClient( 0 );

    LBASSERT( !server->isConnected( ));
    _impl->localServers.erase( i );
    _impl->queue.flush();
    return success;
}

namespace
{
bool _isParameterOption( const std::string& name, const int argc, char** argv,
                         const int index )
{
    return ( index < argc-1 &&          // has enough total arguments
             name == argv[index] &&     // name matches
             argv[index+1][0] != '-' ); // next arg not an option
}
}

bool Client::initLocal( const int argc, char** argv )
{
    bool isClient = false;
    std::string clientOpts;

    if( _impl->name.empty() && argc > 0 && argv )
    {
        const boost::filesystem::path prog = argv[0];
        setName( prog.stem().string( ));
    }

    for( int i=1; i<argc; ++i )
    {
        if( std::string( "--eq-client" ) == argv[i] )
            isClient = true;
        else if( _isParameterOption( "--eq-layout", argc, argv, i ))
            _impl->activeLayouts.push_back( argv[++i] );
        else if( _isParameterOption( "--eq-gpufilter" , argc, argv, i ))
            _impl->gpuFilter = argv[ ++i ];
        else if( _isParameterOption( "--eq-modelunit", argc, argv, i ))
        {
            std::istringstream unitString( argv[++i] );
            unitString >> _impl->modelUnit;
        }
    }
    LBVERB << "Launching " << getNodeID() << std::endl;

    if( !Super::initLocal( argc, argv ))
        return false;

    if( isClient )
    {
        LBVERB << "Client node started from command line with option "
               << clientOpts << std::endl;

        if( !_setupClient( clientOpts ))
        {
            exitLocal();
            return false;
        }

        _impl->running = true;
        clientLoop();
        exitClient();
    }

    _impl->initQt( argc, argv );
    return true;
}

bool Client::_setupClient( const std::string& clientArgs )
{
    LBASSERT( isListening( ));
    if( clientArgs.empty( ))
        return true;

    size_t nextPos = clientArgs.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        LBERROR << "Could not parse working directory: " << clientArgs
                << std::endl;
        return false;
    }

    const std::string workDir = clientArgs.substr( 0, nextPos );
    std::string description = clientArgs.substr( nextPos + 1 );

    Global::setWorkDir( workDir );
    if( !workDir.empty() && chdir( workDir.c_str( )) == -1 )
        LBWARN << "Can't change working directory to " << workDir << ": "
               << lunchbox::sysError << std::endl;

    nextPos = description.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        LBERROR << "Could not parse server node type: " << description
                << " is left from " << clientArgs << std::endl;
        return false;
    }

    co::NodePtr server = createNode( fabric::NODETYPE_SERVER );
    if( !server->deserialize( description ))
        LBWARN << "Can't parse server data" << std::endl;

    LBASSERTINFO( description.empty(), description );
    if( !connect( server ))
    {
        LBERROR << "Can't connect server node using " << *server << std::endl;
        return false;
    }

    return true;
}

void Client::clientLoop()
{
    LBINFO << "Entered client loop" << std::endl;
    while( isRunning( ))
        processCommand();
}

bool Client::exitLocal()
{
#ifdef EQ_QT_USED
    delete _impl->qtApp;
    _impl->qtApp = 0;
#endif
    _impl->activeLayouts.clear();
    _impl->modelUnit = EQ_UNDEFINED_UNIT;
    return fabric::Client::exitLocal();
}

void Client::exitClient()
{
    _impl->queue.flush();
    bool ret = exitLocal();
    LBINFO << "Exit " << lunchbox::className( this ) << " process used "
           << getRefCount() << std::endl;

    if( !eq::exit( ))
        ret = false;
    ::exit( ret ? EXIT_SUCCESS : EXIT_FAILURE );
}

bool Client::hasCommands()
{
    return !_impl->queue.isEmpty();
}

bool Client::isRunning() const
{
    return _impl->running;
}

co::CommandQueue* Client::getMainThreadQueue()
{
    return &_impl->queue;
}

void Client::addActiveLayout( const std::string& activeLayout )
{
    _impl->activeLayouts.push_back( activeLayout );
}

void Client::setName( const std::string& name )
{
    _impl->name = name;
}

const std::string& Client::getName() const
{
    return _impl->name;
}

const Strings& Client::getActiveLayouts() const
{
    return _impl->activeLayouts;
}

const std::string& Client::getGPUFilter() const
{
    return _impl->gpuFilter;
}

float Client::getModelUnit() const
{
    return _impl->modelUnit;
}

void Client::interruptMainThread()
{
    send( fabric::CMD_CLIENT_INTERRUPT );
}

co::NodePtr Client::createNode( const uint32_t type )
{
    switch( type )
    {
        case fabric::NODETYPE_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return Super::createNode( type );
    }
}

bool Client::_cmdExit( co::ICommand& command )
{
    _impl->running = false;
    // Close connection here, this is the last command we'll get on it
    command.getLocalNode()->disconnect( command.getRemoteNode( ));
    return true;
}

bool Client::_cmdInterrupt( co::ICommand& )
{
    return true;
}

namespace
{
class StopNodesVisitor : public ServerVisitor
{
public:
    virtual ~StopNodesVisitor() {}
    virtual VisitorResult visitPre( Node* node )
    {
        node->dirtyClientExit();
        return TRAVERSE_CONTINUE;
    }
};
}

void Client::notifyDisconnect( co::NodePtr node )
{
    if( node->getType() == fabric::NODETYPE_SERVER )
    {
        // local command dispatching
        co::OCommand( this, this, fabric::CMD_CLIENT_EXIT,
                      co::COMMANDTYPE_NODE );

        ServerPtr server = static_cast< Server* >( node.get( ));
        StopNodesVisitor stopNodes;
        server->accept( stopNodes );
    }
    fabric::Client::notifyDisconnect( node );
}

}
