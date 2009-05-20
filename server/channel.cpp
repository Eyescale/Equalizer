
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <pthread.h>
#include "channel.h"

#include "channelListener.h"
#include "channelUpdateVisitor.h"
#include "channelVisitor.h"
#include "compound.h"
#include "compoundVisitor.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "paths.h"
#include "segment.h"
#include "view.h"
#include "window.h"

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/net/command.h>
#include <eq/client/commands.h>
#include <eq/client/global.h>
#include <eq/client/log.h>
#include <eq/client/packets.h>
#include <eq/client/view.h>

#include "channel.ipp"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Channel> ChannelFunc;

void Channel::_construct()
{
    _active           = 0;
    _view             = 0;
    _segment          = 0;
    _window           = 0;
    _fixedPVP         = false;
    _lastDrawCompound = 0;
    _near             = .1f;
    _far              = 10.f;
    _overdraw         = vmml::Vector4i::ZERO;

    _drawable         = 0;
    _tasks            = eq::TASK_NONE;
    EQINFO << "New channel @" << (void*)this << endl;
}

Channel::Channel()
{
    _construct();

    const Global* global = Global::instance();
    
    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _iAttributes[i] = global->getChannelIAttribute(
            static_cast<eq::Channel::IAttribute>( i ));
}

Channel::Channel( const Channel& from, Window* window )
        : net::Object()
{
    _construct();

    _name     = from._name;
    _vp       = from._vp;
    _pvp      = from._pvp;
    _fixedPVP = from._fixedPVP;
    _drawable = from._drawable;
    // Don't copy view and segment. Will be re-set by segment copy ctor

    window->addChannel( this );

    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];
}

void Channel::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* serverQueue  = getServerThreadQueue();
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( eq::CMD_CHANNEL_CONFIG_INIT_REPLY, 
                     ChannelFunc( this, &Channel::_cmdConfigInitReply ),
                     commandQueue );
    registerCommand( eq::CMD_CHANNEL_CONFIG_EXIT_REPLY,
                     ChannelFunc( this, &Channel::_cmdConfigExitReply ),
                     commandQueue );
    registerCommand( eq::CMD_CHANNEL_SET_NEARFAR,
                     ChannelFunc( this, &Channel::_cmdSetNearFar ),
                     commandQueue );
    registerCommand( eq::CMD_CHANNEL_FRAME_FINISH_REPLY,
                     ChannelFunc( this, &Channel::_cmdFrameFinishReply ),
                     serverQueue );
}

Channel::~Channel()
{
    EQINFO << "Delete channel @" << (void*)this << endl;

    if( _window )
        _window->removeChannel( this );
}
void Channel::setDrawable( const uint32_t drawable ) 
{ 
    _drawable = drawable; 
}

Config* Channel::getConfig()
{
    EQASSERT( _window );
    return _window ? _window->getConfig() : 0; 
}

const Config* Channel::getConfig() const
{
    EQASSERT( _window );
    return _window ? _window->getConfig() : 0;
}

Node* Channel::getNode() 
{ 
    EQASSERT( _window );
    return _window ? _window->getNode() : 0;
}

const Node* Channel::getNode() const
{ 
    EQASSERT( _window );
    return _window ? _window->getNode() : 0;
}

Pipe* Channel::getPipe() 
{ 
    EQASSERT( _window );
    return _window ? _window->getPipe() : 0;
}

const Pipe* Channel::getPipe() const
{ 
    EQASSERT( _window );
    return _window ? _window->getPipe() : 0;
}

ChannelPath Channel::getPath() const
{
    EQASSERT( _window );
    ChannelPath path( _window->getPath( ));
    
    const ChannelVector&   channels = _window->getChannels();
    ChannelVector::const_iterator i = std::find( channels.begin(),
                                                 channels.end(), this );
    EQASSERT( i != channels.end( ));
    path.channelIndex = std::distance( channels.begin(), i );
    return path;
}

const CompoundVector& Channel::getCompounds() const
{ 
    return getConfig()->getCompounds();
}

net::CommandQueue* Channel::getServerThreadQueue()
{
    EQASSERT( _window );
    return _window->getServerThreadQueue(); 
}

net::CommandQueue* Channel::getCommandThreadQueue()
{
    EQASSERT( _window );
    return _window->getCommandThreadQueue(); 
}

VisitorResult Channel::accept( ChannelVisitor& visitor )
{
    return visitor.visit( this );
}

VisitorResult Channel::accept( ConstChannelVisitor& visitor ) const
{
    return visitor.visit( this );
}

void Channel::activate()
{ 
    EQASSERT( _window );

    ++_active;
    if( _window ) 
        _window->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Channel::deactivate()
{ 
    EQASSERT( _active != 0 );
    EQASSERT( _window );

    --_active; 
    if( _window ) 
        _window->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
}

void Channel::setOutput( View* view, Segment* segment )
{
    EQASSERT( !_view && !_segment );
    EQASSERT( view && segment );

    _view    = view;
    _segment = segment;

    view->addChannel( this );
    segment->addDestinationChannel( this );
}

void Channel::unsetOutput()
{
    EQASSERT( _view && _segment );

    EQCHECK( _view->removeChannel( this ));
    EQCHECK( _segment->removeDestinationChannel( this ));

    _view    = 0;
    _segment = 0;
}

const Layout* Channel::getLayout() const
{
    EQASSERT( _view );
    return _view ? _view->getLayout() : 0;
}

void Channel::addTasks( const uint32_t tasks )
{
    EQASSERT( _window );
    _tasks |= tasks;
    _window->addTasks( tasks );
}

vmml::Vector3ub Channel::_getUniqueColor() const
{
    vmml::Vector3ub color = vmml::Vector3ub::ZERO;
    uint32_t  value = (reinterpret_cast< size_t >( this ) & 0xffffffffu);

    for( unsigned i=0; i<8; ++i )
    {
        color.r |= ( value&1 << (7-i) ); value >>= 1;
        color.g |= ( value&1 << (7-i) ); value >>= 1;
        color.b |= ( value&1 << (7-i) ); value >>= 1;
    }
    
    return color;
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Channel::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( !pvp.isValid( ))
        return;
    
    _fixedPVP = true;

    if( pvp == _pvp )
        return;

    _pvp = pvp;
    _vp.invalidate();
    notifyViewportChanged();
}

void Channel::setViewport( const eq::Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
     
    _fixedPVP = false;

    if( vp == _vp )
        return;

    _vp = vp;
    _pvp.invalidate();
    notifyViewportChanged();
}

void Channel::notifyViewportChanged()
{
    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedPVP ) // update viewport
        _vp = _pvp.getSubVP( windowPVP );
    else            // update pixel viewport
        _pvp = windowPVP.getSubPVP( _vp );

    EQINFO << "Channel viewport update: " << _pvp << ":" << _vp << endl;
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// update running entities (init/exit)
//---------------------------------------------------------------------------

void Channel::updateRunning( const uint32_t initID )
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return;

    if( isActive() && _state != STATE_RUNNING ) // becoming active
        _configInit( initID );

    if( !isActive( )) // becoming inactive
        _configExit();
}

bool Channel::syncRunning()
{
    bool success = true;
    if( isActive() && _state != STATE_RUNNING  && !_syncConfigInit( ))
        // becoming active
        success = false;

    if( !isActive() && _state != STATE_STOPPED && !_syncConfigExit( ))
        // becoming inactive
        success = false;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_STOPPED || 
              _state == STATE_INIT_FAILED );
    return success;
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Channel::_configInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state         = STATE_INITIALIZING;

    getConfig()->registerObject( this );

    eq::WindowCreateChannelPacket createChannelPacket;
    createChannelPacket.channelID = getID();
    _window->send( createChannelPacket );

    eq::ChannelConfigInitPacket packet;
    packet.initID = initID;
    packet.view   = _view;
    packet.color  = _getUniqueColor();
    packet.tasks  = _tasks;
    packet.drawable = _drawable;
    
    if( _fixedPVP )
        packet.pvp    = _pvp; 
    else
        packet.vp     = _vp;
    memcpy( packet.iAttributes, _iAttributes, 
            eq::Channel::IATTR_ALL * sizeof( int32_t )); 

    EQLOG( LOG_INIT ) << "Init channel" << std::endl;
    send( packet, _name );
}

bool Channel::_syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    const bool success = ( _state == STATE_INIT_SUCCESS );
    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Channel initialization failed: " << _error << endl;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Channel::_configExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit channel" << std::endl;
    eq::ChannelConfigExitPacket packet;
    send( packet );

    eq::WindowDestroyChannelPacket destroyChannelPacket;
    destroyChannelPacket.channelID = getID();
    _window->send( destroyChannelPacket );
}

bool Channel::_syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    getConfig()->deregisterObject( this );

    _state = STATE_STOPPED;
    _tasks = eq::TASK_NONE;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Channel::_setupRenderContext( const uint32_t frameID, 
                                   eq::RenderContext& context )
{
    context.frameID       = frameID;
    context.pvp           = _pvp;
    context.view          = _view;
    context.overdraw      = _overdraw;

    if( !_view || !_segment )
        context.vp            = _vp;
    else
        context.vp.applyView( _segment->getViewport(), _view->getViewport(),
                              _pvp, _overdraw );
}

bool Channel::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    eq::ChannelFrameStartPacket startPacket;
    startPacket.frameNumber = frameNumber;
    _setupRenderContext( frameID, startPacket.context );

    send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK channel " << _name << " start frame  " 
                           << &startPacket << endl;

    bool updated = false;
    const CompoundVector& compounds = getCompounds();

    for( CompoundVector::const_iterator i = compounds.begin(); 
         i != compounds.end(); ++i )
    {
        Compound* compound = *i;
        ChannelUpdateVisitor visitor( this, frameID, frameNumber );

        visitor.setEye( eq::EYE_CYCLOP );
        compound->accept( visitor );

        visitor.setEye( eq::EYE_LEFT );
        compound->accept( visitor );

        visitor.setEye( eq::EYE_RIGHT );
        compound->accept( visitor );
        
        updated |= visitor.isUpdated();
    }

    eq::ChannelFrameFinishPacket finishPacket;
    finishPacket.frameNumber = frameNumber;
    finishPacket.context = startPacket.context;

    send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK channel " << _name << " finish frame  "
                           << &finishPacket << endl;
    _lastDrawCompound = 0;

    return updated;
}

void Channel::send( net::ObjectPacket& packet ) 
{ 
    packet.objectID = getID();
    getNode()->send( packet );
}

void Channel::send( net::ObjectPacket& packet, const std::string& string ) 
{
    packet.objectID = getID();
    getNode()->send( packet, string ); 
}

//---------------------------------------------------------------------------
// Listener interface
//---------------------------------------------------------------------------
void Channel::addListener( ChannelListener* listener )
{
    CHECK_THREAD( _serverThread );

    _listeners.push_back( listener );
}

void Channel::removeListener(  ChannelListener* listener )
{
    ChannelListeners::iterator i = find( _listeners.begin(), _listeners.end(),
                                          listener );
    if( i != _listeners.end( ))
        _listeners.erase( i );
}

void Channel::_fireLoadData( const uint32_t frameNumber, 
                             const uint32_t nStatistics,
                             const eq::Statistic* statistics )
{
    CHECK_THREAD( _serverThread );

    for( ChannelListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyLoadData( this, frameNumber, nStatistics, statistics );
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Channel::_cmdConfigInitReply( net::Command& command ) 
{
    const eq::ChannelConfigInitReplyPacket* packet = 
        command.getPacket<eq::ChannelConfigInitReplyPacket>();
    EQVERB << "handle channel configInit reply " << packet << endl;

    _near  = packet->nearPlane;
    _far   = packet->farPlane;
    _error = packet->error;

    if( packet->result )
        _state = STATE_INIT_SUCCESS;
    else
        _state = STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdConfigExitReply( net::Command& command ) 
{
    const eq::ChannelConfigExitReplyPacket* packet = 
        command.getPacket<eq::ChannelConfigExitReplyPacket>();
    EQVERB << "handle channel configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_EXIT_SUCCESS;
    else
        _state = STATE_EXIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdSetNearFar( net::Command& command )
{
    const eq::ChannelSetNearFarPacket* packet = 
        command.getPacket<eq::ChannelSetNearFarPacket>();
    _near = packet->nearPlane;
    _far  = packet->farPlane;
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameFinishReply( net::Command& command )
{
    const eq::ChannelFrameFinishReplyPacket* packet = 
        command.getPacket<eq::ChannelFrameFinishReplyPacket>();

    // output received events
    for( uint32_t i = 0; i<packet->nStatistics; ++i )
    {
        const eq::Statistic& data = packet->statistics[i];
        EQLOG( eq::LOG_STATS ) << data << endl;
    }

    _fireLoadData(packet->frameNumber, packet->nStatistics, packet->statistics);

    return net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Channel* channel)
{
    if( !channel )
        return os;
    
    os << disableFlush << disableHeader << "channel" << endl;
    os << "{" << endl << indent;

    const std::string& name = channel->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    const Segment* segment = channel->getSegment();
    const View*    view    = channel->getView();
    if( view && segment )
    {
        os << "path     ( ";

        const Config* config = channel->getConfig();
        const std::string& segmentName = segment->getName();
        if( !segmentName.empty() && 
            config->findSegment( segmentName ) == segment )
        {
            os << "segment \"" << segmentName << "\" ";
        }
        else
            os << segment->getPath() << ' ';
        
        const std::string& viewName = view->getName();
        if( !viewName.empty() && 
            config->findView( viewName ) == view )
        {
            os << "view \"" << viewName << '\"';
        }
        else
            os << view->getPath();
        
        os << " )" << endl; 
    }

    const eq::Viewport& vp  = channel->getViewport();
    if( vp.isValid( ) && !channel->_fixedPVP )
    {
        if( vp != eq::Viewport::FULL )
            os << "viewport " << vp << endl;
    }
    else
    {
        const eq::PixelViewport& pvp = channel->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << endl;
    }


    const uint32_t drawable = channel->getDrawable();
    if (drawable !=  eq::Channel::FBO_NONE)
    {
        os << "drawable [";
        
        if ((drawable &  eq::Channel::FBO_COLOR) != 0 )
        {
           os << " FBO_COLOR";
        }
        
        if ((drawable &  eq::Channel::FBO_DEPTH) != 0)
        {
           os << " FBO_DEPTH"; 
        } 
        if ((drawable &  eq::Channel::FBO_STENCIL) != 0) 
        {
           os << " FBO_STENCIL";  
        }
        
        os << " ]" << endl;
    }
    bool attrPrinted   = false;
    
    for( eq::Channel::IAttribute i = static_cast<eq::Channel::IAttribute>( 0 );
         i<eq::Channel::IATTR_ALL; 
         i = static_cast<eq::Channel::IAttribute>( static_cast<uint32_t>(i)+1 ))
    {
        const int value = channel->getIAttribute( i );
        if( value == Global::instance()->getChannelIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
            attrPrinted = true;
        }
        
        os << ( i==eq::Channel::IATTR_HINT_STATISTICS ?
                    "hint_statistics   " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}

}
}
