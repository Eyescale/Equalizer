
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "window.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>

#include <co/objectICommand.h>

namespace eq
{
namespace server
{

typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;
typedef co::CommandFunc<Pipe> PipeFunc;

Pipe::Pipe( Node* parent )
        : Super( parent )
        , _active( 0 )
        , _state( STATE_STOPPED )
        , _lastDrawWindow( 0 )
{
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_LAST; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getPipeIAttribute( attr ));
    }
}

Pipe::~Pipe()
{
}

void Pipe::attach( const UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* cmdQ = getCommandThreadQueue();
    registerCommand( fabric::CMD_OBJECT_SYNC,
                     PipeFunc( this, &Pipe::_cmdSync ), cmdQ );
    registerCommand( fabric::CMD_PIPE_CONFIG_INIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigInitReply ), cmdQ );
    registerCommand( fabric::CMD_PIPE_CONFIG_EXIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigExitReply ), cmdQ );
}

void Pipe::removeChild( const UUID& id )
{
    LBASSERT( getConfig()->isRunning( ));

    Window* window = _findWindow( id );
    LBASSERT( window );
    if( window )
        window->postDelete();
}

ServerPtr Pipe::getServer()
{
    Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getServer() : 0);
}

ConstServerPtr Pipe::getServer() const
{
    const Node* node = getNode();
    LBASSERT( node );
    return node ? node->getServer() : 0;
}

Config* Pipe::getConfig()
{
    Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getConfig() : 0);
}

const Config* Pipe::getConfig() const
{
    const Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getConfig() : 0);
}

co::CommandQueue* Pipe::getMainThreadQueue()
{
    Node* node = getNode();
    LBASSERT( node );
    return node->getMainThreadQueue();
}

co::CommandQueue* Pipe::getCommandThreadQueue()
{
    Node* node = getNode();
    LBASSERT( node );
    return node->getCommandThreadQueue();
}

Channel* Pipe::getChannel( const ChannelPath& path )
{
    const Windows& windows = getWindows();
    LBASSERTINFO( windows.size() > path.windowIndex,
                  "Path " << path << " for " << *this );

    if( windows.size() <= path.windowIndex )
        return 0;

    return windows[ path.windowIndex ]->getChannel( path );
}

void Pipe::activate()
{
    Node* node = getNode();
    LBASSERT( node );

    ++_active;
    if( node )
        node->activate();

    LBLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Pipe::deactivate()
{
    LBASSERT( _active != 0 );

    Node* node = getNode();
    LBASSERT( node );

    --_active;
    if( node )
        node->deactivate();

    LBLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Pipe::addTasks( const uint32_t tasks )
{
    Node* node = getNode();
    LBASSERT( node );
    setTasks( getTasks() | tasks );
    node->addTasks( tasks );
}

co::ObjectOCommand Pipe::send( const uint32_t cmd )
{
    return getNode()->send( cmd, getID( ));
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Pipe::configInit( const uint128_t& initID, const uint32_t frameNumber )
{
    LBASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    LBLOG( LOG_INIT ) << "Create pipe" << std::endl;
    getNode()->send( fabric::CMD_NODE_CREATE_PIPE, getNode()->getID( ))
            << getID() << isThreaded();

    LBLOG( LOG_INIT ) << "Init pipe" << std::endl;
    send( fabric::CMD_PIPE_CONFIG_INIT ) << initID << frameNumber;
}

bool Pipe::syncConfigInit()
{
    LBASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    LBWARN << "Pipe initialization failed" << std::endl;
    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Pipe::configExit()
{
    if( _state == STATE_EXITING )
        return;

    LBASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    LBLOG( LOG_INIT ) << "Exit pipe" << std::endl;
    send( fabric::CMD_PIPE_CONFIG_EXIT );
}

bool Pipe::syncConfigExit()
{
    LBASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS ||
              _state == STATE_EXIT_FAILED );

    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    LBASSERT( success || _state == STATE_EXIT_FAILED );

    _state = isActive() ? STATE_FAILED : STATE_STOPPED;
    setTasks( fabric::TASK_NONE );
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint128_t& frameID, const uint32_t frameNumber )
{
    if( !isRunning( ))
        return;

    LBASSERT( isActive( ))
    send( fabric::CMD_PIPE_FRAME_START_CLOCK );

    send( fabric::CMD_PIPE_FRAME_START )
            << getVersion() << frameID << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK pipe start frame " << frameNumber << " id "
                       << frameID << std::endl;

    const Windows& windows = getWindows();
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
        (*i)->updateDraw( frameID, frameNumber );

    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
        (*i)->updatePost( frameID, frameNumber );

    if( !_lastDrawWindow ) // no FrameDrawFinish sent
    {
        send( fabric::CMD_PIPE_FRAME_DRAW_FINISH ) << frameID << frameNumber;
        LBLOG( LOG_TASKS ) << "TASK pipe draw finish " << getName()
                           << " frame " << frameNumber
                           << " id " << frameID << std::endl;
    }
    _lastDrawWindow = 0;

    send( fabric::CMD_PIPE_FRAME_FINISH ) << frameID << frameNumber;

    LBLOG( LOG_TASKS ) << "TASK pipe finish frame " << frameNumber
                       << " id " << frameID << std::endl;
}



//===========================================================================
// command handling
//===========================================================================
bool Pipe::_cmdConfigInitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const bool result = command.get< bool >();

    LBVERB << "handle pipe configInit reply " << command << " result " << result
           << std::endl;

    _state = result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Pipe::_cmdConfigExitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBVERB << "handle pipe configExit reply " << command << std::endl;

    _state = command.get< bool >() ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

void Pipe::output( std::ostream& os ) const
{
    bool attrPrinted   = false;
    for( IAttribute i = static_cast<IAttribute>( 0 );
         i < IATTR_LAST;
         i = static_cast<IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getPipeIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << lunchbox::indent;
            attrPrinted = true;
        }

        os << ( i == IATTR_HINT_THREAD ? "hint_thread "                   :
                i == IATTR_HINT_CUDA_GL_INTEROP ? "hint_cuda_GL_interop " :
                i == IATTR_HINT_AFFINITY ? "hint_affinity "               :
                    "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    if( attrPrinted )
        os << lunchbox::exdent << "}" << std::endl;
}

}
}

#include "../fabric/pipe.ipp"
template class eq::fabric::Pipe< eq::server::Node, eq::server::Pipe,
                                 eq::server::Window, eq::server::PipeVisitor >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
