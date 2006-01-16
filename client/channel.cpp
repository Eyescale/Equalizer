
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "channel.h"

using namespace eq;
using namespace std;

Channel::Channel()
        : eqNet::Base( CMD_CHANNEL_ALL ),
          _window(NULL)
{
    registerCommand( CMD_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_reqInit ));
    registerCommand( CMD_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqExit ));
    registerCommand( CMD_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqClear ));
}

Channel::~Channel()
{
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::clear()
{
    applyBuffer();
    applyViewport();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Channel::applyBuffer()
{
    glDrawBuffer( _drawBuffer );
}

void Channel::applyViewport()
{
    glViewport( _pvp.x, _pvp.y, _pvp.w, _pvp.h );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Channel::_pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    Pipe* pipe = getPipe();

    if( pipe )
        pipe->pushRequest( node, packet );
    else
        _cmdUnknown( node, packet );
}

void Channel::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelInitPacket* packet = (ChannelInitPacket*)pkg;
    INFO << "handle channel init " << packet << endl;

    ChannelInitReplyPacket reply( packet );
    reply.result = init(); // XXX push to channel thread
    node->send( reply );
}

void Channel::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelExitPacket* packet = (ChannelExitPacket*)pkg;
    INFO << "handle channel exit " << packet << endl;

    exit();

    ChannelExitReplyPacket reply( packet );
    node->send( reply );
}

void Channel::_reqClear( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    INFO << "handle channel clear " << packet << endl;

    _drawBuffer = packet->buffer;
    _pvp        = packet->pvp;

    clear();
}
