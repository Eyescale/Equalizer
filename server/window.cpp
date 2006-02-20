
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "channel.h"
#include "config.h"
#include "pipe.h"

using namespace eqs;
using namespace std;

void Window::_construct()
{
    _used             = 0;
    _pipe             = NULL;
    _pendingRequestID = INVALID_ID;
    _swapMaster       = NULL;
    _swapBarrier      = NULL;

    registerCommand( eq::CMD_WINDOW_INIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(&eqs::Window::_cmdInitReply));
    registerCommand( eq::CMD_WINDOW_EXIT_REPLY, this, 
                     reinterpret_cast<CommandFcn>(&eqs::Window::_cmdExitReply));
}

Window::Window()
        : eqNet::Base( eq::CMD_WINDOW_ALL )
{
    _construct();
}

Window::Window( const Window& from )
        : eqNet::Base( eq::CMD_WINDOW_ALL )
{
    _construct();

    const uint32_t nChannels = from.nChannels();
    for( uint32_t i=0; i<nChannels; i++ )
    {
        Channel* channel      = from.getChannel(i);
        Channel* channelClone = new Channel( *channel );

        addChannel( channelClone );
    }            
}

void Window::addChannel( Channel* channel )
{
    _channels.push_back( channel ); 
    channel->_window = this; 

    Config* config = getConfig();
    if( config )
        config->registerObject( channel );
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

//---------------------------------------------------------------------------
// swap group operations
//---------------------------------------------------------------------------
void Window::resetSwapGroup()
{
    const uint32_t nMembers = _swapGroup.size();
    for( uint32_t i=0; i<nMembers; i++ )
        _swapGroup[i]->_swapMaster = NULL;

    _swapGroup.clear();
    _swapMaster  = NULL;
}

void Window::setSwapGroup( Window* master )
{
    if( _swapMaster )
    {
        EQWARN << "Window already belongs to swap group on " << _swapMaster
             << ", ignoring swap group request." << endl;
        return;
    }

    master->_swapGroup.push_back( this );
    _swapMaster = master;
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Window::startInit()
{
    _sendInit();
    eq::WindowCreateChannelPacket createChannelPacket( getSession()->getID(), 
                                                       getID( ));
    const int nChannels = _channels.size();
    for( int i=0; i<nChannels; ++i )
    {
        Channel* channel = _channels[i];
        if( channel->isUsed( ))
        {
            createChannelPacket.channelID = channel->getID();
            _send( createChannelPacket );
            channel->startInit();
        }
    }
}

void Window::_sendInit()
{
    EQASSERT( _pendingRequestID == INVALID_ID );

    eq::WindowInitPacket packet( getSession()->getID(), getID() );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Window::syncInit()
{
    bool success = true;
    const int nChannels = _channels.size();
    for( int i=0; i<nChannels; ++i )
    {
        Channel* channel = _channels[i];
        if( channel->isUsed( ))
            if( !channel->syncInit( ))
                success = false;
    }

    EQASSERT( _pendingRequestID != INVALID_ID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;
    _pendingRequestID = INVALID_ID;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Window::startExit()
{
    const int nChannels = _channels.size();
    for( int i=0; i<nChannels; ++i )
    {
        Channel* channel = _channels[i];
        if( channel->isUsed( ))
            channel->startExit();
    }

    _sendExit();
}

void Window::_sendExit()
{
    EQASSERT( _pendingRequestID == INVALID_ID );

    eq::WindowExitPacket packet( getSession()->getID(), getID() );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Window::syncExit()
{
    EQASSERT( _pendingRequestID != INVALID_ID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    
    eq::WindowDestroyChannelPacket destroyChannelPacket(getSession()->getID(), 
                                                        getID( ));

    const int nChannels = _channels.size();
    for( int i=0; i<nChannels; ++i )
    {
        Channel* channel = _channels[i];
        if( channel->isUsed( ))
        {
            if( !channel->syncExit( ))
                success = false;

            destroyChannelPacket.channelID = channel->getID();
            _send( destroyChannelPacket );
        }
    }

    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Window::update()
{
    // TODO: send update window task (make current)

    const uint32_t nChannels = this->nChannels();
    for( uint32_t i=0; i<nChannels; i++ )
    {
        Channel* channel = getChannel( i );
        if( channel->isUsed( ))
            channel->update();
    }

    if( _swapMaster ) // swap barrier
    {
        const uint32_t height = _swapMaster->_swapGroup.size();
        if( height > 1 )
        {
            if( _swapMaster == this )
            {
                Node* node = getNode();
            
                if( _swapBarrier && _swapBarrier->getHeight() != height )
                {
                    node->releaseBarrier( _swapBarrier );
                    _swapBarrier = NULL;
                }

                if( !_swapBarrier )
                    _swapBarrier = node->getBarrier( height );
            }

            EQASSERT( _swapMaster->_swapBarrier );

            eq::WindowSwapWithBarrierPacket packet( getSession()->getID(), 
                                                    getID( ));
            packet.barrierID = _swapMaster->_swapBarrier->getID();
            _send( packet );
            return;
        }

        EQWARN << "Swap group of size " << height << ", ignoring request" 
               << endl;
    }

    eq::WindowSwapPacket packet( getSession()->getID(), getID() );
    _send( packet );
}


//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Window::_cmdInitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::WindowInitReplyPacket* packet = (eq::WindowInitReplyPacket*)pkg;
    EQINFO << "handle window init reply " << packet << endl;

    _pvp = packet->pvp;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdExitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::WindowExitReplyPacket* packet = (eq::WindowExitReplyPacket*)pkg;
    EQINFO << "handle window exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}


std::ostream& eqs::operator << ( std::ostream& os, const Window* window )
{
    if( !window )
    {
        os << "NULL window";
        return os;
    }
    
    const uint32_t nChannels = window->nChannels();
    os << "window " << (void*)window
       << ( window->isUsed() ? " used " : " unused " ) << nChannels
       << " channels";
    
    for( uint32_t i=0; i<nChannels; i++ )
        os << std::endl << "    " << window->getChannel(i);
    
    return os;
}
