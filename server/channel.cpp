
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "channel.h"

#include "channelUpdateVisitor.h"
#include "compound.h"
#include "config.h"
#include "global.h"
#include "window.h"

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/net/command.h>
#include <eq/client/commands.h>
#include <eq/client/global.h>
#include <eq/client/log.h>
#include <eq/client/packets.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Channel::_construct()
{
    _used             = 0;
    _window           = NULL;
    _fixedPVP         = false;
    _lastDrawCompound = 0;

    registerCommand( eq::CMD_CHANNEL_CONFIG_INIT_REPLY, 
            eqNet::CommandFunc<Channel>( this, &Channel::_cmdConfigInitReply ));
    registerCommand( eq::CMD_CHANNEL_CONFIG_EXIT_REPLY,
            eqNet::CommandFunc<Channel>( this, &Channel::_cmdConfigExitReply ));
    registerCommand( eq::CMD_CHANNEL_SET_NEARFAR,
                     eqNet::CommandFunc<Channel>( this, &Channel::_cmdPush ));
    registerCommand( eq::REQ_CHANNEL_SET_NEARFAR,
                 eqNet::CommandFunc<Channel>( this, &Channel::_reqSetNearFar ));

    const Global* global = Global::instance();
    
    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _iAttributes[i] = global->getChannelIAttribute(
            static_cast<eq::Channel::IAttribute>( i ));

    EQINFO << "New channel @" << (void*)this << endl;
}

Channel::Channel()
{
    _construct();
}

Channel::Channel( const Channel& from )
{
    _construct();

    _name     = from._name;
    _vp       = from._vp;
    _pvp      = from._pvp;
    _fixedPVP = from._fixedPVP;

    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];
}

Channel::~Channel()
{
    EQINFO << "Delete channel @" << (void*)this << endl;

    if( _window )
        _window->removeChannel( this );
}

void Channel::refUsed()
{
    _used++;
    if( _window ) 
        _window->refUsed(); 
}

void Channel::unrefUsed()
{
    EQASSERT( _used != 0 );
    _used--;
    if( _window ) 
        _window->unrefUsed(); 
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
    if( !pvp.hasArea( ))
        return;
    
    _fixedPVP = true;

    if( pvp == _pvp )
        return;

    _pvp = pvp;
    _vp.invalidate();
    notifyViewportChanged();
    _firePVPChanged();
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
    if( !windowPVP.hasArea( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedPVP ) // update viewport
        _vp = _pvp.getSubVP( windowPVP );
    else            // update pixel viewport
    {
        eq::PixelViewport pvp = windowPVP.getSubPVP( _vp );
        if( _pvp != pvp )
        {
            _pvp = pvp;
            _firePVPChanged();
        }
    }

    EQINFO << "Channel viewport update: " << _pvp << ":" << _vp << endl;
}

void Channel::addPVPListener( PixelViewportListener* listener )
{
    _pvpListeners.push_back( listener );
}

void Channel::removePVPListener( PixelViewportListener* listener )
{
    vector< PixelViewportListener *>::iterator i = find( _pvpListeners.begin(),
                                                         _pvpListeners.end(),
                                                         listener );
    if( i != _pvpListeners.end( ))
        _pvpListeners.erase( i );
}

void Channel::_firePVPChanged()
{
    for( vector<PixelViewportListener*>::const_iterator i=_pvpListeners.begin();
         i != _pvpListeners.end(); ++i )

        (*i)->notifyPVPChanged( _pvp );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// configInit
//---------------------------------------------------------------------------
void Channel::startConfigInit( const uint32_t initID )
{
    _sendConfigInit( initID );
}

void Channel::_sendConfigInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    eq::ChannelConfigInitPacket packet;
    packet.initID     = initID;
    packet.color      = _getUniqueColor();
    if( _fixedPVP )
        packet.pvp    = _pvp; 
    else
        packet.vp     = _vp;

    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        packet.iattr[i] = _iAttributes[i];
    
    send( packet, _name );
    EQLOG( eq::LOG_TASKS ) << "TASK channel configInit  " << &packet << endl;
}

bool Channel::syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_RUNNING ||
              _state == STATE_INIT_FAILED );
    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_RUNNING )
        return true;

    EQWARN << "Channel initialisation failed: " << _error << endl;
    return false;
}

//---------------------------------------------------------------------------
// configExit
//---------------------------------------------------------------------------
void Channel::startConfigExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_STOPPING;
    _sendConfigExit();
}

void Channel::_sendConfigExit()
{
    eq::ChannelConfigExitPacket packet;
    send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK configExit  " << &packet << endl;
}

bool Channel::syncConfigExit()
{
    EQASSERT( _state == STATE_STOPPING || _state == STATE_STOPPED || 
              _state == STATE_STOP_FAILED );
    
    _state.waitNE( STATE_STOPPING );
    if( _state == STATE_STOPPED )
        return true;

    EQASSERT( _state == STATE_STOP_FAILED );
    _state = STATE_STOPPED; /// STOP_FAILED -> STOPPED transition
    return false;
}


//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Channel::updateDraw( const uint32_t frameID, const uint32_t frameNumber )
{
    if( !_lastDrawCompound )
    {
        Config* config = getConfig();
        _lastDrawCompound = config->getCompound(0);
    }

    eq::ChannelFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK channel start frame  " << &startPacket
                           << endl;

    Config*        config     = getConfig();
    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound = config->getCompound( i );
        ChannelUpdateVisitor visitor( this, frameID, frameNumber );

        visitor.setEye( eq::EYE_LEFT );
        compound->accept( &visitor );

        visitor.setEye( eq::EYE_RIGHT );
        compound->accept( &visitor );

        visitor.setEye( eq::EYE_CYCLOP );
        compound->accept( &visitor );
    }
}

void Channel::updatePost( const uint32_t frameID, const uint32_t frameNumber )
{
    eq::ChannelFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK channel finish frame  " << &finishPacket
                           << endl;
    _lastDrawCompound = 0;
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Channel::_cmdConfigInitReply( eqNet::Command& command ) 
{
    const eq::ChannelConfigInitReplyPacket* packet = 
        command.getPacket<eq::ChannelConfigInitReplyPacket>();
    EQINFO << "handle channel configInit reply " << packet << endl;

    _near  = packet->nearPlane;
    _far   = packet->farPlane;
    _error = packet->error;

    if( packet->result )
        _state = STATE_RUNNING;
    else
        _state = STATE_INIT_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdConfigExitReply( eqNet::Command& command ) 
{
    const eq::ChannelConfigExitReplyPacket* packet = 
        command.getPacket<eq::ChannelConfigExitReplyPacket>();
    EQINFO << "handle channel configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_STOPPED;
    else
        _state = STATE_STOP_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqSetNearFar( eqNet::Command& command )
{
    const eq::ChannelSetNearFarPacket* packet = 
        command.getPacket<eq::ChannelSetNearFarPacket>();
    _near = packet->nearPlane;
    _far  = packet->farPlane;
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eqs::operator << ( std::ostream& os, const Channel* channel)
{
    if( !channel )
        return os;
    
    os << disableFlush << disableHeader << "channel" << endl;
    os << "{" << endl << indent;
    
    const std::string& name = channel->getName();
    if( name.empty( ))
        os << "name     \"channel_" << (void*)channel << "\"" << endl;
    else
        os << "name     \"" << name << "\"" << endl;

    const eq::Viewport& vp  = channel->getViewport();
    if( vp.isValid( ) && !channel->_fixedPVP )
    {
        if( !vp.isFullScreen( ))
            os << "viewport " << vp << endl;
    }
    else
    {
        const eq::PixelViewport& pvp = channel->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << endl;
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
