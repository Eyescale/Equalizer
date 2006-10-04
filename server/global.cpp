
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

using namespace eqs;
using namespace eqBase;
using namespace std;

static Global *_instance = NULL;

Global* Global::instance()
{
    if( !_instance ) 
        _instance = new Global();

    return _instance;
}

Global::Global()
{
    for( int i=0; i<Node::IATTR_ALL; ++i )
        _nodeIAttributes[i] = eq::UNDEFINED;

    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
        _connectionIAttributes[i] = eq::UNDEFINED;

    _connectionIAttributes[ConnectionDescription::IATTR_TYPE] = 
        eqNet::Connection::TYPE_TCPIP;
    _connectionIAttributes[ConnectionDescription::IATTR_TCPIP_PORT] = 0;
    
    _connectionSAttributes[ConnectionDescription::SATTR_HOSTNAME] = "localhost";
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c >& %h.%n.log";
        
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _windowIAttributes[i] = eq::UNDEFINED;

    _windowIAttributes[eq::Window::IATTR_HINTS_STEREO]       = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINTS_DOUBLEBUFFER] = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_PLANES_COLOR]       = 1;
    _windowIAttributes[eq::Window::IATTR_PLANES_DEPTH]       = 1;
    
    _configFAttributes[Config::FATTR_EYE_BASE] = 0.05;
}

std::ostream& eqs::operator << ( std::ostream& os, const Global* global )
{
    Global reference;

    os << disableFlush << disableHeader << "global" << endl;
    os << '{' << indent << endl;
#if 0
    for( int i=0; i<Node::IATTR_ALL; ++i )
    {
        if( global->_nodeIAttributes[i] != reference._nodeIAttributes[i] )
        {
        }
    }

    for( int i=0; i<Node::SATTR_ALL; ++i )
    {
        if( global->_nodeSAttributes[i] != reference._nodeSAttributes[i] )
        {
        }
    }
#endif

    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const int value = global->_connectionIAttributes[i];
        if( value == reference._connectionIAttributes[i] )
            continue;

        os << ( i==ConnectionDescription::IATTR_TYPE ?
                "EQ_CONNECTION_TYPE           " :
                i==ConnectionDescription::IATTR_TCPIP_PORT ?
                "EQ_CONNECTION_TCPIP_PORT     " :
                "EQ_CONNECTION_LAUNCH_TIMEOUT " );
                
        switch( i )
        { 
            case ConnectionDescription::IATTR_TYPE:
                os << ( value == eqNet::Connection::TYPE_TCPIP ? "TCPIP" : 
                        "PIPE" );
                break;
            case ConnectionDescription::IATTR_LAUNCH_TIMEOUT:
                os << value;// << " ms";
                break;
            default:
                os << value;
        }
        os << endl;
    }

    for( int i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const string& value = global->_connectionSAttributes[i];
        if( value == reference._connectionSAttributes[i] )
            continue;

        os << ( i==ConnectionDescription::SATTR_HOSTNAME ?
                "EQ_CONNECTION_HOSTNAME       " :
                "EQ_CONNECTION_LAUNCH_COMMAND " )
           << "\"" << value << "\"" << endl;
    }

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const int value = global->_windowIAttributes[i];
        if( value == reference._windowIAttributes[i] )
            continue;

        os << ( i==eq::Window::IATTR_HINTS_STEREO ?
                    "EQ_WINDOW_IATTR_HINTS_STEREO       " :
                i==eq::Window::IATTR_HINTS_DOUBLEBUFFER ?
                    "EQ_WINDOW_IATTR_HINTS_DOUBLEBUFFER " :
                i==eq::Window::IATTR_PLANES_COLOR ? 
                    "EQ_WINDOW_IATTR_PLANES_COLOR       " :
                i==eq::Window::IATTR_PLANES_ALPHA ?
                    "EQ_WINDOW_IATTR_PLANES_ALPHA       " :
                i==eq::Window::IATTR_PLANES_DEPTH ?
                    "EQ_WINDOW_IATTR_PLANES_DEPTH       " :
                i==eq::Window::IATTR_PLANES_STENCIL ?
                    "EQ_WINDOW_IATTR_PLANES_STENCIL     " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( int i=0; i<Config::FATTR_ALL; ++i )
    {
        const float value = global->_configFAttributes[i];
        if( value != reference._configFAttributes[i] )
            continue;

        os << ( i==Config::FATTR_EYE_BASE ?
                    "EQ_CONFIG_FATTR_EYE_BASE           " : "ERROR" )
           << value << endl;
    }

    os << exdent << '}' << endl << enableHeader << enableFlush;
    return os;
}

