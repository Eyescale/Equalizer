
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

#include "colorMask.h"

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
    // connection
    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
        _connectionIAttributes[i] = eq::UNDEFINED;

    _connectionIAttributes[ConnectionDescription::IATTR_TYPE] = 
        eqNet::CONNECTIONTYPE_TCPIP;
    _connectionIAttributes[ConnectionDescription::IATTR_TCPIP_PORT] = 0;
    _connectionIAttributes[ConnectionDescription::IATTR_LAUNCH_TIMEOUT] = 
        10000; // ms
    
    _connectionSAttributes[ConnectionDescription::SATTR_HOSTNAME] = "localhost";
#ifdef WIN32
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c";
#else
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c >& %h.%n.log";
#endif

    // config
    _configFAttributes[Config::FATTR_EYE_BASE] = 0.05f;

    // pipe
    for( int i=0; i<Pipe::IATTR_ALL; ++i )
        _pipeIAttributes[i] = eq::UNDEFINED;

    _pipeIAttributes[Pipe::IATTR_HINT_THREAD] = eq::ON;

    // window
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _windowIAttributes[i] = eq::UNDEFINED;

    _windowIAttributes[eq::Window::IATTR_HINT_STEREO]       = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_DOUBLEBUFFER] = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_FULLSCREEN]   = eq::OFF;
    _windowIAttributes[eq::Window::IATTR_HINT_DECORATION]   = eq::ON;
    _windowIAttributes[eq::Window::IATTR_PLANES_COLOR]       = 1;
    _windowIAttributes[eq::Window::IATTR_PLANES_DEPTH]       = 1;
    
    // channel
    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
        _channelIAttributes[i] = eq::UNDEFINED;

    _channelIAttributes[eq::Channel::IATTR_HINT_STATISTICS] = eq::FASTEST;

    // compound
    for( int i=0; i<Compound::IATTR_ALL; ++i )
        _compoundIAttributes[i] = eq::UNDEFINED;
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
    for( int i=0; i<Pipe::IATTR_ALL; ++i )
    {
        const string& name     = Pipe::getIAttributeString(
            (Pipe::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _pipeIAttributes[i] = atol( envValue );
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
    for( int i=0; i<Compound::IATTR_ALL; ++i )
    {
        const string& name     = Compound::getIAttributeString(
            (Compound::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _compoundIAttributes[i] = atol( envValue );
    }
}

#define GLOBAL_ATTR_LENGTH 50

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

        const string& name = ConnectionDescription::getIAttributeString( 
            static_cast<ConnectionDescription::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' );
                
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

        const string& name = ConnectionDescription::getSAttributeString( 
            static_cast<ConnectionDescription::SAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "\"" << value << "\"" << endl;
    }

    for( int i=0; i<Config::FATTR_ALL; ++i )
    {
        const float value = global->_configFAttributes[i];
        if( value == reference._configFAttributes[i] )
            continue;

        const string& name = Config::getFAttributeString( 
            static_cast<Config::FAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << value << endl;
    }

    for( int i=0; i<Pipe::IATTR_ALL; ++i )
    {
        const int value = global->_pipeIAttributes[i];
        if( value == reference._pipeIAttributes[i] )
            continue;

        const string& name = Pipe::getIAttributeString( 
            static_cast<Pipe::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const int value = global->_windowIAttributes[i];
        if( value == reference._windowIAttributes[i] )
            continue;

        const string& name = eq::Window::getIAttributeString( 
            static_cast<eq::Window::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( int i=0; i<eq::Channel::IATTR_ALL; ++i )
    {
        const int value = global->_channelIAttributes[i];
        if( value == reference._channelIAttributes[i] )
            continue;

        const string& name = eq::Channel::getIAttributeString(
            static_cast<eq::Channel::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( int i=0; i<Compound::IATTR_ALL; ++i )
    {
        const int value = global->_compoundIAttributes[i];
        if( value == reference._compoundIAttributes[i] )
            continue;

        const string& name = Compound::getIAttributeString(
            static_cast<Compound::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' );

        switch( i )
        {
            case Compound::IATTR_STEREO_MODE:
                os << static_cast<eq::IAttrValue>( value ) << endl;
                break;

            case Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK:
            case Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK:
                os << ColorMask( value ) << endl;
                break;

            default:
                EQASSERTINFO( 0, "unimplemented" );
        }
    }

    os << exdent << '}' << endl << enableHeader << enableFlush;
    return os;
}

