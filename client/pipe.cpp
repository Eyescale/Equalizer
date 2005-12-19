
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "window.h"

using namespace eq;
using namespace std;

Pipe::Pipe()
        : eqNet::Base( CMD_PIPE_ALL ),
          _node(NULL)
{
    registerCommand( CMD_PIPE_CREATE_WINDOW, this, reinterpret_cast<CommandFcn>(
                         &eq::Pipe::_cmdCreateWindow ));
    registerCommand( CMD_PIPE_DESTROY_WINDOW, this,reinterpret_cast<CommandFcn>(
                         &eq::Pipe::_cmdDestroyWindow ));
    registerCommand( CMD_PIPE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Pipe::_cmdInit ));
    registerCommand( REQ_PIPE_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Pipe::_reqInit ));
    registerCommand( CMD_PIPE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Pipe::pushRequest ));
    registerCommand( REQ_PIPE_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Pipe::_reqExit ));

    _thread = new PipeThread( this );
}

Pipe::~Pipe()
{
    delete _thread; 
}

void Pipe::_addWindow( Window* window )
{
    _windows.push_back( window );
    window->_pipe = this;
}

void Pipe::_removeWindow( Window* window )
{
    vector<Window*>::iterator iter = find( _windows.begin(), _windows.end(),
                                           window );
    if( iter == _windows.end( ))
        return;
    
    _windows.erase( iter );
    window->_pipe = NULL;
}

ssize_t Pipe::_runThread()
{
    Config* config = getConfig();
    ASSERT( config );

    Node::setLocalNode( config->getNode( ));

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _thread->isRunning( ))
    {
        _requestQueue.pop( &node, &packet );
        config->dispatchPacket( node, packet );
    }

    _thread->join();
    return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Pipe::_cmdCreateWindow( eqNet::Node* node, const eqNet::Packet* pkg )
{
    PipeCreateWindowPacket* packet = (PipeCreateWindowPacket*)pkg;
    INFO << "Handle create window " << packet << endl;

    Window* window = Global::getNodeFactory()->createWindow();
    
    getConfig()->addRegisteredObject( packet->windowID, window );
    _addWindow( window );
}

void Pipe::_cmdDestroyWindow( eqNet::Node* node, const eqNet::Packet* pkg )
{
    PipeDestroyWindowPacket* packet = (PipeDestroyWindowPacket*)pkg;
    INFO << "Handle destroy window " << packet << endl;

    Config* config = getConfig();
    Window* window = (Window*)config->getRegisteredObject( packet->windowID );
    if( !window )
        return;

    _removeWindow( window );
    config->deregisterObject( window );
    delete window;
}

void Pipe::_cmdInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    PipeInitPacket* packet = (PipeInitPacket*)pkg;
    INFO << "handle pipe init (recv)" << packet << endl;

    ASSERT( _thread->isStopped( ));
    _thread->start();
    pushRequest( node, pkg );
}

void Pipe::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    PipeInitPacket* packet = (PipeInitPacket*)pkg;
    INFO << "handle pipe init (pipe)" << packet << endl;
    PipeInitReplyPacket reply( packet );
    reply.result = init();
    node->send( reply );
}

void Pipe::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    PipeExitPacket* packet = (PipeExitPacket*)pkg;
    INFO << "handle pipe exit " << packet << endl;

    exit();
    
    PipeExitReplyPacket reply( packet );
    node->send( reply );

    _thread->exit( EXIT_SUCCESS );
}
