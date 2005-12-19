
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "channel.h"

using namespace eq;
using namespace std;

Window::Window()
        : eqNet::Base( CMD_WINDOW_ALL ),
          _pipe(NULL)
{
    registerCommand( CMD_WINDOW_CREATE_CHANNEL, this,
                     reinterpret_cast<CommandFcn>(
                         &eq::Window::_cmdCreateChannel ));
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL, this,
                     reinterpret_cast<CommandFcn>(
                         &eq::Window::_cmdDestroyChannel ));
    registerCommand( CMD_WINDOW_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_pushRequest ));
    registerCommand( REQ_WINDOW_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_reqInit ));
    registerCommand( CMD_WINDOW_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Window::_pushRequest ));
    registerCommand( REQ_WINDOW_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Window::_reqExit ));
}

Window::~Window()
{
}

void Window::_addChannel( Channel* channel )
{
    _channels.push_back( channel );
    channel->_window = this;
}

void Window::_removeChannel( Channel* channel )
{
    vector<Channel*>::iterator iter = find( _channels.begin(), _channels.end(), 
                                            channel );
    if( iter == _channels.end( ))
        return;
    
    _channels.erase( iter );
    channel->_window = NULL;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Window::_pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    if( _pipe )
        _pipe->pushRequest( node, packet );
    else
        _cmdUnknown( node, packet );
}

void Window::_cmdCreateChannel( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowCreateChannelPacket* packet = (WindowCreateChannelPacket*)pkg;
    INFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel();
    
    getConfig()->addRegisteredObject( packet->channelID, channel );
    _addChannel( channel );
}

void Window::_cmdDestroyChannel( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowDestroyChannelPacket* packet = (WindowDestroyChannelPacket*)pkg;
    INFO << "Handle destroy channel " << packet << endl;

    Config*  config  = getConfig();
    Channel* channel = (Channel*)config->getRegisteredObject(packet->channelID);
    if( !channel )
        return;

    _removeChannel( channel );
    config->deregisterObject( channel );
    delete channel;
}

void Window::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowInitPacket* packet = (WindowInitPacket*)pkg;
    INFO << "handle window init " << packet << endl;

    WindowInitReplyPacket reply( packet );
    reply.result = init();
    node->send( reply );
}

void Window::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowExitPacket* packet = (WindowExitPacket*)pkg;
    INFO << "handle window exit " << packet << endl;

    exit();

    WindowExitReplyPacket reply( packet );
    node->send( reply );
}
