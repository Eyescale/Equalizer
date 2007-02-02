
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "global.h"
#include "channel.h"
#include "config.h"
#include "compound.h"
#include "log.h"
#include "pipe.h"

#include <eq/net/command.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void eqs::Window::_construct()
{
    _used             = 0;
    _pipe             = NULL;
    _pendingRequestID = EQ_ID_INVALID;
    _state            = STATE_STOPPED;

    registerCommand( eq::CMD_WINDOW_INIT_REPLY, 
                     eqNet::CommandFunc<Window>( this, &Window::_cmdInitReply));
    registerCommand( eq::CMD_WINDOW_EXIT_REPLY, 
                     eqNet::CommandFunc<Window>( this, &Window::_cmdExitReply));
    registerCommand( eq::CMD_WINDOW_SET_PVP, 
                     eqNet::CommandFunc<Window>( this, &Window::_cmdPushFront));
    registerCommand( eq::REQ_WINDOW_SET_PVP,
             eqNet::CommandFunc<Window>( this, &Window::_reqSetPixelViewport ));
                         
    ref(); // We don't use RefPtr so far
    EQINFO << "New window @" << (void*)this << endl;
}

eqs::Window::Window()
{
    _construct();
    
    const Global* global = Global::instance();
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = global->getWindowIAttribute(
            static_cast<eq::Window::IAttribute>( i ));

}

eqs::Window::Window( const Window& from )
{
    _construct();

    _name = from._name;
    _pvp  = from._pvp;
    _vp   = from._vp;
    
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    const uint32_t nChannels = from.nChannels();
    for( uint32_t i=0; i<nChannels; i++ )
    {
        Channel* channel      = from.getChannel(i);
        Channel* channelClone = new Channel( *channel );

        addChannel( channelClone );
    }            
}

eqs::Window::~Window()
{
    EQINFO << "Delete window @" << (void*)this << endl;

    if( _pipe )
        _pipe->removeWindow( this );
    
    for( vector<Channel*>::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        EQASSERT( channel->getRefCount() == 1 );

        channel->_window = NULL;
        channel->unref(); // a.k.a delete
    }
    _channels.clear();
}


void eqs::Window::addChannel( Channel* channel )
{
    _channels.push_back( channel ); 
    channel->_window = this; 
}

bool eqs::Window::removeChannel( Channel* channel )
{
    vector<Channel*>::iterator i = find( _channels.begin(), _channels.end(),
                                        channel );
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    channel->_window = 0;
    return true;
}

void eqs::Window::refUsed()
{
    _used++;
    if( _pipe ) 
        _pipe->refUsed(); 
}
void eqs::Window::unrefUsed()
{
    _used--;
    if( _pipe ) 
        _pipe->unrefUsed(); 
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void eqs::Window::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return;

    _pvp = pvp;
    _vp.invalidate();

    if( _pipe )
    {
        const eq::PixelViewport& pipePVP = _pipe->getPixelViewport();
        if( pipePVP.isValid( ))
            _vp = pvp / pipePVP;
    }

    EQINFO << "Window pvp set: " << _pvp << ":" << _vp << endl;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )

        (*iter)->notifyViewportChanged();
}

void eqs::Window::setViewport( const eq::Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _vp = vp;

    if( !_pipe )
        return;

    eq::PixelViewport pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
    {
        pipePVP.x = 0;
        pipePVP.y = 0;
        _pvp = pipePVP * vp;
    }
}

void eqs::Window::notifyViewportChanged()
{
    if( !_pipe || !_pvp.hasArea( ))
        return;

    const eq::PixelViewport& pipePVP = _pipe->getPixelViewport();
    if( !pipePVP.hasArea( ))
        return;

    // We always assume that the window's pvp is fixed
    _vp = _pvp / pipePVP;
    EQINFO << "Window viewport update: " << _pvp << ":" << _vp << endl;
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void eqs::Window::_resetSwapBarriers()
{ 
    Node* node = getNode();
    EQASSERT( node );

    for( vector<eqNet::Barrier*>::iterator iter = _masterSwapBarriers.begin();
         iter != _masterSwapBarriers.end(); ++iter )
            
        node->releaseBarrier( *iter );

    _masterSwapBarriers.clear();
    _swapBarriers.clear();
}

eqNet::Barrier* eqs::Window::newSwapBarrier()
{
    Node*           node    = getNode();
    eqNet::Barrier* barrier = node->getBarrier();
    _masterSwapBarriers.push_back( barrier );

    joinSwapBarrier( barrier );
    return barrier;
}

void eqs::Window::joinSwapBarrier( eqNet::Barrier* barrier )
{ 
    barrier->increase();
    _swapBarriers.push_back( barrier );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void eqs::Window::startInit( const uint32_t initID )
{
    _sendInit( initID );

    Config* config = getConfig();
    eq::WindowCreateChannelPacket createChannelPacket;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->isUsed( ))
        {
            config->registerObject( channel, (eqNet::Node*)getServer( ));
            createChannelPacket.channelID = channel->getID();
            _send( createChannelPacket );

            channel->startInit( initID );
        }
    }
    _state = STATE_INITIALISING;
}

void eqs::Window::_sendInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::WindowInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 

    packet.requestID = _pendingRequestID;
    packet.initID    = initID;
    packet.pvp       = _pvp; 
    packet.vp        = _vp;
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        packet.iattr[i] = _iAttributes[i];
    
    _send( packet, getName( ));
    EQLOG( eq::LOG_TASKS ) << "TASK init  " << &packet << endl;
}

bool eqs::Window::syncInit()
{
    bool success = true;
    string error;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->isUsed( ))
            if( !channel->syncInit( ))
            {
                error += (' ' + channel->getErrorMessage());
                success = false;
            }
    }

    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    if( !static_cast<bool>( _requestHandler.waitRequest( _pendingRequestID )))
        success = false;

    _pendingRequestID = EQ_ID_INVALID;
    _error += (' ' + error);

    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Window initialisation failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void eqs::Window::startExit()
{
    _state = STATE_STOPPING;
    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->getState() == Channel::STATE_STOPPED )
            continue;

        channel->startExit();
    }

    _sendExit();
}

void eqs::Window::_sendExit()
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::WindowExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK exit  " << &packet << endl;
}

bool eqs::Window::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;
    
    Config* config = getConfig();
    eq::WindowDestroyChannelPacket destroyChannelPacket;

    for( std::vector<Channel*>::iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->getState() != Channel::STATE_STOPPING )
            continue;

        if( !channel->syncExit( ))
            success = false;

        destroyChannelPacket.channelID = channel->getID();
        _send( destroyChannelPacket );
        config->deregisterObject( channel );
    }

    _state = STATE_STOPPED;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void eqs::Window::update( const uint32_t frameID )
{
    // TODO: make current window
    eq::WindowStartFramePacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.makeCurrent = _pipe->testMakeCurrentWindow( this );
    _send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK start frame  " << &startPacket << endl;

    const uint32_t nChannels = this->nChannels();
    for( uint32_t i=0; i<nChannels; i++ )
    {
        Channel* channel = getChannel( i );
        if( channel->isUsed( ))
            channel->update( frameID );
    }

    _updateSwap();

    eq::WindowEndFramePacket endPacket;
    endPacket.frameID = frameID;
    _send( endPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK end frame  " << &endPacket << endl;
}

void eqs::Window::_updateSwap()
{
    if( !_swapBarriers.empty())
    {
        eq::WindowFinishPacket packet;
        _send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK finish  " << &packet << endl;
    }

    for( vector<eqNet::Barrier*>::iterator iter = _swapBarriers.begin();
         iter != _swapBarriers.end(); ++iter )
    {
        const eqNet::Barrier*   barrier = *iter;
        eq::WindowBarrierPacket packet;

        packet.barrierID      = barrier->getID();
        packet.barrierVersion = barrier->getVersion();
        _send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK barrier  " << &packet << endl;
    }

    eq::WindowSwapPacket packet;
    _send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK swap  " << &packet << endl;
    _resetSwapBarriers();
}


//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult eqs::Window::_cmdInitReply( eqNet::Command& command )
{
    const eq::WindowInitReplyPacket* packet =
        command.getPacket<eq::WindowInitReplyPacket>();
    EQINFO << "handle window init reply " << packet << endl;

    if( packet->result )
        _drawableConfig = packet->drawableConfig;
    
    if( packet->pvp.isValid( ))
        setPixelViewport( packet->pvp );

    _error = packet->error;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eqs::Window::_cmdExitReply( eqNet::Command& command )
{
    const eq::WindowExitReplyPacket* packet =
        command.getPacket<eq::WindowExitReplyPacket>();
    EQINFO << "handle window exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eqs::Window::_reqSetPixelViewport( eqNet::Command& command)
{
    const eq::WindowSetPVPPacket* packet = 
        command.getPacket<eq::WindowSetPVPPacket>();
    EQVERB << "handle window set pvp " << packet << endl;

    setPixelViewport( packet->pvp );
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eqs::operator << ( std::ostream& os, const eqs::Window* window )
{
    if( !window )
        return os;
    
    os << disableFlush << disableHeader << "window" << endl;
    os << "{" << endl << indent; 

    const std::string& name = window->getName();
    if( name.empty( ))
        os << "name \"window_" << (void*)window << "\"" << endl;
    else
        os << "name \"" << name << "\"" << endl;

    const eq::Viewport& vp  = window->getViewport();
    if( vp.isValid( ))
    {
        if( !vp.isFullScreen( ))
            os << "viewport " << vp << endl;
    }
    else
    {
        const eq::PixelViewport& pvp = window->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << endl;
    }

    bool attrPrinted   = false;
    
    for( eq::Window::IAttribute i = static_cast<eq::Window::IAttribute>( 0 );
         i<eq::Window::IATTR_ALL; 
         i = static_cast<eq::Window::IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = window->getIAttribute( i );
        if( value == Global::instance()->getWindowIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
            attrPrinted = true;
        }
        
        os << ( i==eq::Window::IATTR_HINT_STEREO ?
                    "hint_stereo       " :
                i==eq::Window::IATTR_HINT_DOUBLEBUFFER ?
                    "hint_doublebuffer " :
                i==eq::Window::IATTR_HINT_FULLSCREEN ?
                    "hint_fullscreen   " :
                i==eq::Window::IATTR_HINT_DECORATION ?
                    "hint_decoration   " :
                i==eq::Window::IATTR_PLANES_COLOR ? 
                    "planes_color      " :
                i==eq::Window::IATTR_PLANES_ALPHA ?
                    "planes_alpha      " :
                i==eq::Window::IATTR_PLANES_DEPTH ?
                    "planes_depth      " :
                i==eq::Window::IATTR_PLANES_STENCIL ?
                    "planes_stencil    " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    const uint32_t nChannels = window->nChannels();
    for( uint32_t i=0; i<nChannels; i++ )
        os << window->getChannel(i);

    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}
