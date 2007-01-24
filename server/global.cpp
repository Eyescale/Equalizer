
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
    _setupDefaults();
    _readEnvironment();
}

void Global::_setupDefaults()
{
    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
        _connectionIAttributes[i] = eq::UNDEFINED;

    _connectionIAttributes[ConnectionDescription::IATTR_TYPE] = 
        eqNet::CONNECTIONTYPE_TCPIP;
    _connectionIAttributes[ConnectionDescription::IATTR_TCPIP_PORT] = 0;
    
    _connectionSAttributes[ConnectionDescription::SATTR_HOSTNAME] = "localhost";
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c >& %h.%n.log";
        
    _configFAttributes[Config::FATTR_EYE_BASE] = 0.05f;

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _windowIAttributes[i] = eq::UNDEFINED;

    _windowIAttributes[eq::Window::IATTR_HINT_STEREO]       = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_DOUBLEBUFFER] = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_FULLSCREEN]   = eq::OFF;
    _windowIAttributes[eq::Window::IATTR_HINT_DECORATION]   = eq::ON;
    _windowIAttributes[eq::Window::IATTR_PLANES_COLOR]       = 1;
    _windowIAttributes[eq::Window::IATTR_PLANES_DEPTH]       = 1;
    
    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _channelIAttributes[i] = eq::UNDEFINED;

    _channelIAttributes[eq::Channel::IATTR_HINT_STATISTICS] = eq::FASTEST;
}

void Global::_readEnvironment()
{
    for( int i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const string& name     = ConnectionDescription::getSAttributeString(
            (ConnectionDescription::SAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionSAttributes[i] = envValue;
    }
    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const string& name     = ConnectionDescription::getIAttributeString(
            (ConnectionDescription::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionIAttributes[i] = atol( envValue );
    }
    for( int i=0; i<Config::FATTR_ALL; ++i )
    {
        const string& name     = Config::getFAttributeString(
            (Config::FAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _configFAttributes[i] = atof( envValue );
    }
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const string& name     = eq::Window::getIAttributeString(
            (eq::Window::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _windowIAttributes[i] = atol( envValue );
    }
    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
    {
        const string& name     = eq::Channel::getIAttributeString(
            (eq::Channel::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _channelIAttributes[i] = atol( envValue );
    }
}

std::ostream& eqs::operator << ( std::ostream& os, const Global* global )
{
    Global reference;
    reference._setupDefaults(); // ignore environment variables

    os << disableFlush << disableHeader << "global" << endl;
    os << '{' << indent << endl;

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
                os << ( value == eqNet::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
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

    for( int i=0; i<Config::FATTR_ALL; ++i )
    {
        const float value = global->_configFAttributes[i];
        if( value == reference._configFAttributes[i] )
            continue;

        os << ( i==Config::FATTR_EYE_BASE ?
                    "EQ_CONFIG_FATTR_EYE_BASE           " : "ERROR" )
           << value << endl;
    }

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const int value = global->_windowIAttributes[i];
        if( value == reference._windowIAttributes[i] )
            continue;

        os << ( i==eq::Window::IATTR_HINT_STEREO ?
                    "EQ_WINDOW_IATTR_HINT_STEREO       " :
                i==eq::Window::IATTR_HINT_DOUBLEBUFFER ?
                    "EQ_WINDOW_IATTR_HINT_DOUBLEBUFFER " :
                i==eq::Window::IATTR_HINT_FULLSCREEN ?
                    "EQ_WINDOW_IATTR_HINT_FULLSCREEN   " :
                i==eq::Window::IATTR_HINT_DECORATION ?
                    "EQ_WINDOW_IATTR_HINT_DECORATION   " :
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

    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
    {
        const int value = global->_channelIAttributes[i];
        if( value == reference._channelIAttributes[i] )
            continue;

        os << ( i==eq::Channel::IATTR_HINT_STATISTICS ?
                    "EQ_CHANNEL_IATTR_HINT_STATISTICS   " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    os << exdent << '}' << endl << enableHeader << enableFlush;
    return os;
}

