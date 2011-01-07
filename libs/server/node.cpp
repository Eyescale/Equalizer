
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "node.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

#include <eq/configPackets.h>
#include <eq/error.h>
#include <eq/nodePackets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <co/barrier.h>
#include <co/command.h>
#include <co/base/launcher.h>
#include <co/base/sleep.h>

namespace eq
{
namespace server
{
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;
typedef co::CommandFunc<Node> NodeFunc;
namespace
{
#define S_MAKE_ATTR_STRING( attr ) ( std::string("EQ_NODE_") + #attr )
std::string _sAttributeStrings[] = {
    S_MAKE_ATTR_STRING( SATTR_LAUNCH_COMMAND )
};
std::string _cAttributeStrings[] = {
    S_MAKE_ATTR_STRING( CATTR_LAUNCH_COMMAND_QUOTE )
};
}

Node::Node( Config* parent )
    : Super( parent ) 
    , _active( 0 )
    , _finishedFrame( 0 )
    , _flushedFrame( 0 )
    , _state( STATE_STOPPED )
    , _lastDrawPipe( 0 )
{
    const Global* global = Global::instance();    
    for( int i=0; i < Node::SATTR_LAST; ++i )
    {
        const SAttribute attr = static_cast< SAttribute >( i );
        setSAttribute( attr, global->getNodeSAttribute( attr ));
    }
    for( int i=0; i < Node::CATTR_LAST; ++i )
    {
        const CAttribute attr = static_cast< CAttribute >( i );
        setCAttribute( attr, global->getNodeCAttribute( attr ));
    }
    for( int i = 0; i < IATTR_LAST; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getNodeIAttribute( attr ));
    }
}

Node::~Node()
{
}

void Node::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    
    co::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( fabric::CMD_NODE_CONFIG_INIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigInitReply ), queue );
    registerCommand( fabric::CMD_NODE_CONFIG_EXIT_REPLY, 
                     NodeFunc( this, &Node::_cmdConfigExitReply ), queue );
    registerCommand( fabric::CMD_NODE_FRAME_FINISH_REPLY,
                     NodeFunc( this, &Node::_cmdFrameFinishReply ), queue );
}

ServerPtr Node::getServer()
{
    return getConfig() ? getConfig()->getServer() : 0;
}

ConstServerPtr Node::getServer() const
{
    return getConfig() ? getConfig()->getServer() : 0;
}

co::CommandQueue* Node::getMainThreadQueue()
{
    return getConfig()->getMainThreadQueue();
}

co::CommandQueue* Node::getCommandThreadQueue()
{
    return getConfig()->getCommandThreadQueue();
}

Channel* Node::getChannel( const ChannelPath& path )
{
    const Pipes& pipes = getPipes();
    EQASSERT( pipes.size() > path.pipeIndex );

    if( pipes.size() <= path.pipeIndex )
        return 0;

    return pipes[ path.pipeIndex ]->getChannel( path );
}

void Node::addTasks( const uint32_t tasks )
{
    setTasks( getTasks() | tasks );
}

void Node::activate()
{   
    ++_active;
    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Node::deactivate()
{ 
    EQASSERT( _active != 0 );
    --_active; 
    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Node::setSAttribute( const SAttribute attr, const std::string& value )
{
    _sAttributes[attr] = value;
}

const std::string& Node::getSAttribute( const SAttribute attr ) const
{
    return _sAttributes[attr];
}

const std::string& Node::getSAttributeString( const SAttribute attr )
{
    return _sAttributeStrings[attr];
}

void Node::setCAttribute( const CAttribute attr, const char value )
{
    _cAttributes[attr] = value;
}

char Node::getCAttribute( const CAttribute attr ) const
{
    return _cAttributes[attr];
}

const std::string& Node::getCAttributeString( const CAttribute attr )
{
    return _cAttributeStrings[attr];
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// launch and connect
//---------------------------------------------------------------------------

namespace
{
static co::NodePtr _createNetNode( Node* node )
{
    co::NodePtr netNode = new co::Node;
    const co::ConnectionDescriptions& descriptions = 
        node->getConnectionDescriptions();
    for( co::ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        netNode->addConnectionDescription( *i );
    }

    return netNode;
}
}

bool Node::connect()
{
    EQASSERT( isActive( ));

    if( _node.isValid( ))
        return _node->isConnected();

    if( !isStopped( ))
    {
        EQASSERT( _state == STATE_FAILED );
        return true;
    }

    co::LocalNodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));
    
    if( !_node )
    {
        EQASSERT( !isApplicationNode( ));
        _node = _createNetNode( this );
    }
    else
    {
        EQASSERT( isApplicationNode( ));
    }

    EQLOG( LOG_INIT ) << "Connecting node" << std::endl;
    if( !localNode->connect( _node ) && !launch( ))
    {
        EQWARN << "Connection to " << _node->getNodeID() << " failed"
               << std::endl;
        _state = STATE_FAILED;
        _node = 0;
        return false;
    }

    return true;
}

bool Node::launch()
{
    for( co::ConnectionDescriptions::const_iterator i = 
             _connectionDescriptions.begin();
         i != _connectionDescriptions.end(); ++i )
    {
        co::ConnectionDescriptionPtr description = *i;
        const std::string launchCommand = _createLaunchCommand( description );
        if( co::base::Launcher::run( launchCommand ))
            return true;

        EQWARN << "Could not launch node using '" << launchCommand << "'" 
               << std::endl;
    }

    setError( ERROR_NODE_LAUNCH );
    return false;
}

bool Node::syncLaunch( const co::base::Clock& clock )
{
    EQASSERT( isActive( ));

    if( !_node )
        return false;

    if( _node->isConnected( ))
        return true;

    EQASSERT( !isApplicationNode( ));
    co::LocalNodePtr localNode = getLocalNode();
    EQASSERT( localNode.isValid( ));

    const int32_t timeOut = getIAttribute( IATTR_LAUNCH_TIMEOUT );

    while( true )
    {
        co::NodePtr node = localNode->getNode( _node->getNodeID( ));
        if( node.isValid() && node->isConnected( ))
        {
            EQASSERT( _node->getRefCount() == 1 );
            _node = node; // Use co::Node already connected
            return true;
        }
        
        co::base::sleep( 100 /*ms*/ );
        if( clock.getTime64() > timeOut )
        {
            EQASSERT( _node->getRefCount() == 1 );
            _node = 0;
            std::ostringstream data;

            for( co::ConnectionDescriptions::const_iterator i =
                     _connectionDescriptions.begin();
                 i != _connectionDescriptions.end(); ++i )
            {
                co::ConnectionDescriptionPtr desc = *i;
                data << desc->getHostname() << ' ';
            }
            setError( ERROR_NODE_CONNECT );
            EQWARN << getError() << std::endl;

            _state = STATE_FAILED;
            return false;
        }
    }
}

std::string Node::_createLaunchCommand(
    co::ConnectionDescriptionPtr description )
{
    const std::string& command = getSAttribute( SATTR_LAUNCH_COMMAND );
    const size_t commandLen = command.size();

    bool commandFound = false;
    size_t lastPos = 0;
    std::string result;

    for( size_t percentPos = command.find( '%' );
         percentPos != std::string::npos; 
         percentPos = command.find( '%', percentPos+1 ))
    {
        std::ostringstream replacement;
        switch( command[percentPos+1] )
        {
            case 'c':
            {
                replacement << _createRemoteCommand();
                commandFound = true;
                break;
            }
            case 'h':
            {
                const std::string& hostname = description->getHostname();
                if( hostname.empty( ))
                    replacement << "127.0.0.1";
                else
                    replacement << hostname;
                break;
            }
            case 'n':
                replacement << _node->getNodeID();
                break;

            default:
                EQWARN << "Unknown token " << command[percentPos+1] 
                       << std::endl;
                replacement << '%' << command[percentPos+1];
        }

        result += command.substr( lastPos, percentPos-lastPos );
        if( !replacement.str().empty( ))
            result += replacement.str();

        lastPos  = percentPos+2;
    }

    result += command.substr( lastPos, commandLen-lastPos );

    if( !commandFound )
        result += " " + _createRemoteCommand();

    EQVERB << "Launch command: " << result << std::endl;
    return result;
}

std::string Node::_createRemoteCommand()
{
    std::ostringstream stringStream;

    //----- environment
#ifndef WIN32
#  ifdef Darwin
    const char libPath[] = "DYLD_LIBRARY_PATH";
#  else
    const char libPath[] = "LD_LIBRARY_PATH";
#  endif

    stringStream << "env "; // XXX
    char* env = getenv( libPath );
    if( env )
        stringStream << libPath << "=" << env << " ";

    for( int i=0; environ[i] != 0; i++ )
        if( strlen( environ[i] ) > 2 && strncmp( environ[i], "EQ_", 3 ) == 0 )
            stringStream << environ[i] << " ";

    stringStream << "EQ_LOG_LEVEL=" <<co::base::Log::getLogLevelString() << " ";
    if( co::base::Log::topics != 0 )
        stringStream << "EQ_LOG_TOPICS=" <<co::base::Log::topics << " ";
#endif // WIN32

    //----- program + args
    const Config* config = getConfig();
    std::string program = config->getRenderClient();
    const std::string& workDir = config->getWorkDir();
#ifdef WIN32
    EQASSERT( program.length() > 2 );
    if( !( program[1] == ':' && (program[2] == '/' || program[2] == '\\' )) &&
        // !( drive letter and full path present )
        !( program[0] == '/' || program[0] == '\\' ))
        // !full path without drive letter
    {
        program = workDir + '/' + program; // add workDir to relative path
    }
#else
    if( program[0] != '/' )
        program = workDir + '/' + program;
#endif

    const char quote = getCAttribute( CATTR_LAUNCH_COMMAND_QUOTE );
    const std::string ownData = getServer()->serialize();
    const std::string remoteData = _node->serialize();

    stringStream
        << quote << program << quote << " -- --eq-client " << quote
        << remoteData << workDir << CO_SEPARATOR << ownData << quote;

    return stringStream.str();
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Node::configInit( const uint128_t& initID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    _flushedFrame  = 0;
    _finishedFrame = 0;
    _frameIDs.clear();

    EQLOG( LOG_INIT ) << "Create node" << std::endl;
    ConfigCreateNodePacket createNodePacket;
    createNodePacket.nodeID = getID();
    getConfig()->send( _node, createNodePacket );

    EQLOG( LOG_INIT ) << "Init node" << std::endl;
    NodeConfigInitPacket packet;
    packet.initID      = initID;
    packet.frameNumber = frameNumber;

    _send( packet );
}

bool Node::syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    EQWARN << "Node initialization failed: " << getError() << std::endl;
    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Node::configExit()
{
    if( _state == STATE_EXITING )
        return;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit node" << std::endl;
    NodeConfigExitPacket packet;
    _send( packet );
    flushSendBuffer();
}

bool Node::syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    _state = isActive() ? STATE_FAILED : STATE_STOPPED;
    setTasks( fabric::TASK_NONE );
    _frameIDs.clear();
    _flushBarriers();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Node::update( const uint128_t& frameID, const uint32_t frameNumber )
{
    EQVERB << "Start frame " << frameNumber << std::endl;
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    _frameIDs[ frameNumber ] = frameID;
    
    NodeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = getVersion();
    if( !isApplicationNode( )) // synced in Config::_cmdFrameStart
        startPacket.configVersion = getConfig()->getVersion();

    _send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK node start frame " << &startPacket << std::endl;

    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        Pipe* pipe = *i;
        if( pipe->isActive() && pipe->isRunning( ))
            pipe->update( frameID, frameNumber );
    }

    NodeFrameTasksFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    _send( finishPacket );
    EQLOG( LOG_TASKS ) << "TASK node tasks finish " << &finishPacket
                           << std::endl;

    _finish( frameNumber );

    flushSendBuffer();
    _lastDrawPipe = 0;
}

uint32_t Node::_getFinishLatency() const
{
    switch( getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case fabric::UNDEFINED:
        case fabric::DRAW_SYNC:
            if( getTasks() & fabric::TASK_DRAW )
            {
                // More than one frame latency doesn't make sense, since the
                // draw sync for frame+1 does not allow for more
                const Config* config = getConfig();
                const uint32_t latency = config->getLatency();

                return EQ_MIN( latency, 1 );
            }
            break;

        case fabric::LOCAL_SYNC:
            if( getTasks() != fabric::TASK_NONE )
                // local sync enforces no latency
                return 0;
            break;

        case fabric::ASYNC:
            break;
        default:
            EQUNIMPLEMENTED;
    }

    const Config* config = getConfig();
    return config->getLatency();
}

void Node::_finish( const uint32_t currentFrame )
{
    const Pipes& pipes = getPipes();
    for( Pipes::const_iterator i = pipes.begin(); i != pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        if( pipe->getIAttribute( Pipe::IATTR_HINT_THREAD ) && pipe->isRunning())
        {
            const uint32_t latency = _getFinishLatency();
            if( currentFrame > latency )
                flushFrames( currentFrame - latency );
            return;
        }
    }

    // else only non-threaded pipes, all local tasks are done, send finish now.
    flushFrames( currentFrame );
}

void Node::flushFrames( const uint32_t frameNumber )
{
    EQLOG( LOG_TASKS ) << "Flush frames including " << frameNumber << std::endl;

    while( _flushedFrame < frameNumber )
    {
        ++_flushedFrame;
        _sendFrameFinish( _flushedFrame );
    }

    flushSendBuffer();
}

void Node::_sendFrameFinish( const uint32_t frameNumber )
{
    FrameIDHash::iterator i = _frameIDs.find( frameNumber );
    if( i == _frameIDs.end( ))
        return; // finish already send

    NodeFrameFinishPacket packet;
    packet.frameID     = i->second;
    packet.frameNumber = frameNumber;

    _send( packet );
    _frameIDs.erase( i );
    EQLOG( LOG_TASKS ) << "TASK node finish frame  " << &packet << std::endl;
}

//---------------------------------------------------------------------------
// Barrier cache
//---------------------------------------------------------------------------
co::Barrier* Node::getBarrier()
{
    if( _barriers.empty() )
        return new co::Barrier( _node );
    // else

    co::Barrier* barrier = _barriers.back();
    _barriers.pop_back();
    barrier->setHeight(0);
    return barrier;
}

void Node::changeLatency( const uint32_t latency )
{
    for( co::Barriers::const_iterator i = _barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        co::Barrier* barrier = *i;
        barrier->setAutoObsolete( latency + 1 );
    }
}

void Node::releaseBarrier( co::Barrier* barrier )
{
    _barriers.push_back( barrier );
}

void Node::_flushBarriers()
{
    for( std::vector< co::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        co::Barrier* barrier = *i;
        getServer()->deregisterObject( barrier );
        delete barrier;
    }
    _barriers.clear();
}

bool Node::removeConnectionDescription( co::ConnectionDescriptionPtr cd )
{
    // Don't use std::find, RefPtr::operator== compares pointers, not values.
    for(co::ConnectionDescriptions::iterator i=_connectionDescriptions.begin();
        i != _connectionDescriptions.end(); ++i )
    {
        if( *cd != **i )
            continue;

        _connectionDescriptions.erase( i );
        return true;
    }
    return false;
}

void Node::flushSendBuffer()
{
    _bufferedTasks.sendBuffer( _node->getConnection( ));
}

//===========================================================================
// command handling
//===========================================================================
bool Node::_cmdConfigInitReply( co::Command& command )
{
    const NodeConfigInitReplyPacket* packet = 
        command.getPacket<NodeConfigInitReplyPacket>();
    EQVERB << "handle configInit reply " << packet << std::endl;
    EQASSERT( _state == STATE_INITIALIZING );
    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;

    return true;
}

bool Node::_cmdConfigExitReply( co::Command& command )
{
    const NodeConfigExitReplyPacket* packet =
        command.getPacket<NodeConfigExitReplyPacket>();
    EQVERB << "handle configExit reply " << packet << std::endl;
    EQASSERT( _state == STATE_EXITING );

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

bool Node::_cmdFrameFinishReply( co::Command& command )
{
    const NodeFrameFinishReplyPacket* packet = 
        command.getPacket<NodeFrameFinishReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << std::endl;
    
    _finishedFrame = packet->frameNumber;
    getConfig()->notifyNodeFrameFinished( packet->frameNumber );

    return true;
}

void Node::output( std::ostream& os ) const
{
    const co::ConnectionDescriptions& descriptions = _connectionDescriptions;
    for( co::ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        co::ConnectionDescriptionPtr desc = *i;
        os << *desc;
    }

    bool attrPrinted   = false;
    
    for( Node::SAttribute i = static_cast<Node::SAttribute>( 0 );
         i < Node::SATTR_LAST;
         i = static_cast<Node::SAttribute>( static_cast<uint32_t>( i )+1))
    {
        const std::string& value = getSAttribute( i );
        if( value == Global::instance()->getNodeSAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Node::SATTR_LAUNCH_COMMAND ? "launch_command       " :
                "ERROR" )
           << "\"" << value << "\"" << std::endl;
    }
    
    for( Node::CAttribute i = static_cast<Node::CAttribute>( 0 );
         i < Node::CATTR_LAST; 
         i = static_cast<Node::CAttribute>( static_cast<uint32_t>( i )+1))
    {
        const char value = getCAttribute( i );
        if( value == Global::instance()->getNodeCAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Node::CATTR_LAUNCH_COMMAND_QUOTE ? "launch_command_quote " :
                "ERROR" )
           << "'" << value << "'" << std::endl;
    }
    
    for( Node::IAttribute i = static_cast< Node::IAttribute>( 0 );
         i < Node::IATTR_LAST;
         i = static_cast< Node::IAttribute >( static_cast<uint32_t>( i )+1))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getNodeIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i== Node::IATTR_LAUNCH_TIMEOUT ? "launch_timeout       " :
                i== Node::IATTR_THREAD_MODEL   ? "thread_model         " :
                "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << co::base::exdent << "}" << std::endl << std::endl;
}

}
}

#include "../fabric/node.ipp"
template class eq::fabric::Node< eq::server::Config, eq::server::Node,
                                 eq::server::Pipe, eq::server::NodeVisitor >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
