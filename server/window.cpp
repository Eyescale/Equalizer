
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "window.h"
#include "global.h"
#include "channel.h"
#include "config.h"
#include "compound.h"
#include "log.h"
#include "pipe.h"

#include <eq/net/command.h>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eqs
{
void Window::_construct()
{
    _used             = 0;
    _pipe             = NULL;
    _fixedPVP         = false;
    _lastDrawCompound = 0;

    EQINFO << "New window @" << (void*)this << endl;
}

Window::Window()
{
    _construct();
    
    const Global* global = Global::instance();
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = global->getWindowIAttribute(
            static_cast<eq::Window::IAttribute>( i ));
}

Window::Window( const Window& from, const CompoundVector& compounds )
        : eqNet::Object()
{
    _construct();

    _name     = from._name;
    _pvp      = from._pvp;
    _vp       = from._vp;
    _fixedPVP = from._fixedPVP;
    
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    const ChannelVector& channels = from.getChannels();
    for( ChannelVector::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        const Channel* channel      = *i;
        Channel*       channelClone = new Channel( *channel, compounds );
        
        addChannel( channelClone );
    }            
}

Window::~Window()
{
    EQINFO << "Delete window @" << (void*)this << endl;

    if( _pipe )
        _pipe->removeWindow( this );
    
    for( vector<Channel*>::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;

        channel->_window = NULL;
        delete channel;
    }
    _channels.clear();
}

void Window::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               eqNet::Session* session )
{
    eqNet::Object::attachToSession( id, instanceID, session );
    
    eqNet::CommandQueue* serverQueue  = getServerThreadQueue();
    eqNet::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( eq::CMD_WINDOW_CONFIG_INIT_REPLY, 
                     CommandFunc<Window>( this, &Window::_cmdConfigInitReply),
                     commandQueue );
    registerCommand( eq::CMD_WINDOW_CONFIG_EXIT_REPLY, 
                     CommandFunc<Window>( this, &Window::_cmdConfigExitReply),
                     commandQueue );
    registerCommand( eq::CMD_WINDOW_SET_PVP, 
                     CommandFunc<Window>( this, &Window::_cmdSetPixelViewport ),
                     serverQueue );
                         
}

void Window::addChannel( Channel* channel )
{
    _channels.push_back( channel ); 
    channel->_window = this;
    channel->notifyViewportChanged();
}

bool Window::removeChannel( Channel* channel )
{
    vector<Channel*>::iterator i = find( _channels.begin(), _channels.end(),
                                        channel );
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    channel->_window = 0;
    return true;
}

void Window::refUsed()
{
    _used++;
    if( _pipe ) 
        _pipe->refUsed(); 
}
void Window::unrefUsed()
{
    _used--;
    if( _pipe ) 
        _pipe->unrefUsed(); 
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Window::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( !pvp.hasArea( ))
        return;

    _fixedPVP = true;

    if( pvp == _pvp )
        return;

    _pvp      = pvp;
    _vp.invalidate();
    notifyViewportChanged();
}

void Window::setViewport( const eq::Viewport& vp )
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

void Window::notifyViewportChanged()
{
    if( _pipe )
    {
        eq::PixelViewport pipePVP = _pipe->getPixelViewport();
        if( pipePVP.hasArea( ))
        {
            if( _fixedPVP ) // update viewport
                _vp = _pvp.getSubVP( pipePVP );
            else            // update pixel viewport
                _pvp = pipePVP.getSubPVP( _vp );
        }
    }
    EQINFO << "Window viewport update: " << _pvp << ":" << _vp << endl;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )

        (*iter)->notifyViewportChanged();
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void Window::_resetSwapBarriers()
{ 
    Node* node = getNode();
    EQASSERT( node );

    for( vector<eqNet::Barrier*>::iterator iter = _masterSwapBarriers.begin();
         iter != _masterSwapBarriers.end(); ++iter )
            
        node->releaseBarrier( *iter );

    _masterSwapBarriers.clear();
    _swapBarriers.clear();
}

eqNet::Barrier* Window::newSwapBarrier()
{
    Node*           node    = getNode();
    eqNet::Barrier* barrier = node->getBarrier();
    _masterSwapBarriers.push_back( barrier );

    joinSwapBarrier( barrier );
    return barrier;
}

void Window::joinSwapBarrier( eqNet::Barrier* barrier )
{ 
    barrier->increase();
    _swapBarriers.push_back( barrier );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// configInit
//---------------------------------------------------------------------------
void Window::startConfigInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    _sendConfigInit( initID );

    Config* config = getConfig();
    eq::WindowCreateChannelPacket createChannelPacket;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->isUsed( ))
        {
            config->registerObject( channel );
            createChannelPacket.channelID = channel->getID();
            _send( createChannelPacket );

            channel->startConfigInit( initID );
        }
    }
}

void Window::_sendConfigInit( const uint32_t initID )
{
    eq::WindowConfigInitPacket packet;
    packet.initID    = initID;
    if( _fixedPVP )
        packet.pvp    = _pvp; 
    else
        packet.vp     = _vp;

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        packet.iattr[i] = _iAttributes[i];
    
    _send( packet, _name );
    EQLOG( eq::LOG_TASKS ) << "TASK window configInit  " << &packet << endl;
}

bool Window::syncConfigInit()
{
    bool success = true;
    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->isUsed( ))
            if( !channel->syncConfigInit( ))
            {
                _error += ", channel " + channel->getName() + ": '"  +
                          channel->getErrorMessage() + '\'';
                success = false;
            }
    }

    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_RUNNING ||
              _state == STATE_INIT_FAILED );
    _state.waitNE( STATE_INITIALIZING );
    if( _state == STATE_INIT_FAILED )
        success = false;

    if( !success )
        EQWARN << "Window initialisation failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// configExit
//---------------------------------------------------------------------------
void Window::startConfigExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_STOPPING;

    for( std::vector<Channel*>::iterator iter = _channels.begin(); 
         iter != _channels.end(); ++iter )
    {
        Channel* channel = *iter;
        if( channel->getState() == Channel::STATE_STOPPED )
            continue;

        channel->startConfigExit();
    }

    _sendConfigExit();
}

void Window::_sendConfigExit()
{
    eq::WindowConfigExitPacket packet;
    _send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK configExit  " << &packet << endl;
}

bool Window::syncConfigExit()
{
    EQASSERT( _state == STATE_STOPPING || _state == STATE_STOPPED || 
              _state == STATE_STOP_FAILED );
    
    _state.waitNE( STATE_STOPPING );
    bool success = ( _state == STATE_STOPPED );
    EQASSERT( success || _state == STATE_STOP_FAILED );
    _state = STATE_STOPPED; /// STOP_FAILED -> STOPPED transition

    Config* config = getConfig();
    eq::WindowDestroyChannelPacket destroyChannelPacket;

    for( std::vector<Channel*>::iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->getID() == EQ_ID_INVALID )
            continue;

        if( !channel->syncConfigExit( ))
            success = false;

        destroyChannelPacket.channelID = channel->getID();
        _send( destroyChannelPacket );
        config->deregisterObject( channel );
    }
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Window::updateDraw( const uint32_t frameID, 
                              const uint32_t frameNumber )
{
    if( !_lastDrawCompound )
    {
        const Config* config = getConfig();
        _lastDrawCompound = config->getCompounds()[ 0 ];
    }

    eq::WindowFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    _send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK window start frame  " << &startPacket 
                           << endl;

    for( vector< Channel* >::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->isUsed( ))
            channel->updateDraw( frameID, frameNumber );
    }
}

void Window::updatePost( const uint32_t frameID, 
                         const uint32_t frameNumber )
{
    for( vector< Channel* >::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->isUsed( ))
            channel->updatePost( frameID, frameNumber );
    }

    _updateSwap( frameNumber );

    eq::WindowFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    _send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK window finish frame  " << &finishPacket
                           << endl;
    _lastDrawCompound = 0;
}

void Window::_updateSwap( const uint32_t frameNumber )
{
    bool firstBarrier = false;

    for( vector<eqNet::Barrier*>::iterator iter = _swapBarriers.begin();
         iter != _swapBarriers.end(); ++iter )
    {
        const eqNet::Barrier*   barrier = *iter;
        if( barrier->getHeight() <= 1 )
        {
            EQWARN << "Ignoring swap barrier of height " << barrier->getHeight()
                   << endl;
            continue;
        }

        if( !firstBarrier )
        {
            eq::WindowFinishPacket packet;
            _send( packet );
            EQLOG( eq::LOG_TASKS ) << "TASK finish  " << &packet << endl;

            updateFrameFinishNT( frameNumber );
            firstBarrier = true;
        }

        eq::WindowBarrierPacket packet;

        packet.barrierID      = barrier->getID();
        packet.barrierVersion = barrier->getVersion();
        _send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK barrier  " << &packet << endl;
    }

    if( !_drawableConfig.doublebuffered )
        return;

    eq::WindowSwapPacket packet;
    _send( packet );
    EQLOG( eq::LOG_TASKS ) << "TASK swap  " << &packet << endl;
    _resetSwapBarriers();
}

void Window::updateFrameFinishNT( const uint32_t currentFrame )
{
    Pipe* pipe = getPipe();
    if( pipe->isThreaded( ))
        return;

    Node* node = pipe->getNode();
    node->updateFrameFinishNT( currentFrame );
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Window::_cmdConfigInitReply( eqNet::Command& command )
{
    const eq::WindowConfigInitReplyPacket* packet =
        command.getPacket<eq::WindowConfigInitReplyPacket>();
    EQINFO << "handle window configInit reply " << packet << endl;

    if( packet->pvp.isValid( ))
        setPixelViewport( packet->pvp );

    _error = packet->error;

    if( packet->result )
    {
        _drawableConfig = packet->drawableConfig;
        _state = STATE_RUNNING;
    }
    else
        _state = STATE_INIT_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdConfigExitReply( eqNet::Command& command )
{
    const eq::WindowConfigExitReplyPacket* packet =
        command.getPacket<eq::WindowConfigExitReplyPacket>();
    EQINFO << "handle window configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_STOPPED;
    else
        _state = STATE_STOP_FAILED;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdSetPixelViewport( eqNet::Command& command)
{
    const eq::WindowSetPVPPacket* packet = 
        command.getPacket<eq::WindowSetPVPPacket>();
    EQVERB << "handle window set pvp " << packet << endl;

    setPixelViewport( packet->pvp );
    return eqNet::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Window* window )
{
    if( !window )
        return os;
    
    os << disableFlush << disableHeader << "window" << endl;
    os << "{" << endl << indent; 

    const std::string& name = window->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    const eq::Viewport& vp  = window->getViewport();
    if( vp.isValid( ) && !window->_fixedPVP )
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
                i==eq::Window::IATTR_HINT_SWAPSYNC ?
                    "hint_swapsync     " :
                i==eq::Window::IATTR_HINT_DRAWABLE ?
                    "hint_drawable     " :
                i==eq::Window::IATTR_HINT_STATISTICS ?
                    "hint_statistics   " :
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

    const ChannelVector& channels = window->getChannels();
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )

        os << *i;

    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}
}
