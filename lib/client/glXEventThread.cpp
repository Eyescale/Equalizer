/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXEventThread.h"

#include <eq/client/commands.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>
#include <eq/client/X11Connection.h>
#include <eq/net/pipeConnection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

GLXEventThread::GLXEventThread()
        : eqNet::Base( CMD_GLXEVENTTHREAD_ALL )
{
    registerCommand( CMD_GLXEVENTTHREAD_ADD_PIPE, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdAddPipe ));
    registerCommand( CMD_GLXEVENTTHREAD_REMOVE_PIPE, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdRemovePipe ));
}

bool GLXEventThread::init()
{
    eqNet::PipeConnection*    pipeConnection = new eqNet::PipeConnection();
    RefPtr<eqNet::Connection> connection     = pipeConnection;

    if( !connection->connect( ))
        return false;

    _commandConnection[EVENT_END] = connection;
    _commandConnection[APP_END]   = pipeConnection->getChildEnd();

    _connections.addConnection( _commandConnection[EVENT_END] );
    return true;
}

void GLXEventThread::exit()
{
    _commandConnection[EVENT_END]->close();
    _commandConnection[APP_END]->close();
    
    _commandConnection[EVENT_END] = NULL;
    _commandConnection[APP_END]   = NULL;
    EventThread::exit();
}

void GLXEventThread::addPipe( Pipe* pipe )
{
    if( isStopped( ))
        start();

    GLXEventThreadAddPipePacket packet;
    packet.pipe = pipe;
    _commandConnection[APP_END]->send( packet );
}

void GLXEventThread::removePipe( Pipe* pipe )
{
    GLXEventThreadRemovePipePacket packet;
    packet.pipe      = pipe;
    packet.requestID = _requestHandler.registerRequest();
    _commandConnection[APP_END]->send( packet );
    
    const bool stop = _requestHandler.waitRequest( packet.requestID );
    if( stop )
        join();
}

//===========================================================================
// Event thread methods
//===========================================================================

ssize_t GLXEventThread::run()
{
    EQINFO << "GLXEventThread running" << endl;

    while( true )
    {
        const eqNet::ConnectionSet::Event event = _connections.select( );
        switch( event )
        {
            case eqNet::ConnectionSet::EVENT_CONNECT:
            case eqNet::ConnectionSet::EVENT_TIMEOUT:   
            EQUNREACHABLE;
            break;

            case eqNet::ConnectionSet::EVENT_DISCONNECT:
            {
                RefPtr<eqNet::Connection> connection = 
                    _connections.getConnection();
                _connections.removeConnection( connection );
                EQERROR << "Display connection shut down" << endl;
                break;
            }
            
            case eqNet::ConnectionSet::EVENT_DATA:      
                _handleEvent();
                break;
                
            case eqNet::ConnectionSet::EVENT_ERROR:      
                EQWARN << "Error during select" << endl;
                break;
                
            default:
                EQUNIMPLEMENTED;
        }
    }
}

void GLXEventThread::_handleEvent()
{
    RefPtr<eqNet::Connection> connection = _connections.getConnection();

    if( connection ==  _commandConnection[EVENT_END] )
    {
        _handleCommand();
        return;
    }
    
    _handleEvent( RefPtr_static_cast<X11Connection, eqNet::Connection>(
                      connection ));
}

void GLXEventThread::_handleCommand()
{
    uint64_t size;
    const uint64_t read = _commandConnection[EVENT_END]->recv( &size, 
                                                               sizeof( size ));
    if( read == 0 ) // Some systems signal data on dead connections.
        return;

    EQASSERT( read == sizeof( size ));
    EQASSERT( size );
    EQASSERT( size < 4096 );

    eqNet::Packet* packet = (eqNet::Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char*      ptr     = (char*)packet + sizeof(size);
    const bool gotData = _commandConnection[EVENT_END]->recv( ptr, size );
    EQASSERT( gotData );

    const eqNet::CommandResult result = handleCommand( NULL, packet );
    EQASSERT( result == eqNet::COMMAND_HANDLED );
}        

void GLXEventThread::_handleEvent( RefPtr<X11Connection> connection )
{
    // process event(s) on display
    EQUNIMPLEMENTED;
}

eqNet::CommandResult GLXEventThread::_cmdAddPipe( eqNet::Node*,
                                                  const eqNet::Packet* pkg )
{
    GLXEventThreadAddPipePacket* packet = (GLXEventThreadAddPipePacket*)pkg;

    Pipe*        pipe        = packet->pipe;
    const string displayName = pipe->getXDisplayString();
    Display* display = XOpenDisplay( displayName.c_str( ));

    if( !display )
    {
        EQERROR << "Can't open display: " << XDisplayName( displayName.c_str( ))
                << endl;
        return eqNet::COMMAND_HANDLED;
    }
    EQINFO << "Start processing events on display " << displayName << endl;

    X11Connection*            x11Connection = new X11Connection( display );
    RefPtr<eqNet::Connection> connection    = x11Connection;

    x11Connection->setUserdata( pipe );
    pipe->setXEventConnection( x11Connection );
    _connections.addConnection( connection );

    return eqNet::COMMAND_HANDLED;    
}

eqNet::CommandResult GLXEventThread::_cmdRemovePipe( eqNet::Node*,
                                                     const eqNet::Packet* pkg )
{
    GLXEventThreadRemovePipePacket* packet = 
        (GLXEventThreadRemovePipePacket*)pkg;

    Pipe*                     pipe          = packet->pipe;
    X11Connection*            x11Connection = pipe->getXEventConnection();
    RefPtr<eqNet::Connection> connection    = x11Connection;
    
    _connections.removeConnection( connection );
    pipe->setXEventConnection( NULL );
    XCloseDisplay( x11Connection->getDisplay( ));

    EQINFO << "Stopped processing events on display "
           << pipe->getXDisplayString() << endl;

    if( _connections.nConnections() > 1 )        
        _requestHandler.serveRequest( packet->requestID, (void*)false );
    else
    {
        _requestHandler.serveRequest( packet->requestID, (void*)true );
        EQINFO << "GLXEventThread exiting" << endl;
        exit();
    }

    return eqNet::COMMAND_HANDLED;    
}

