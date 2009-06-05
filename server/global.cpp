
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "global.h"

#include "colorMask.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

static Global *_instance = 0;

Global* Global::instance()
{
    if( !_instance ) 
        _instance = new Global();

    return _instance;
}

void Global::clear()
{
    delete _instance;
    _instance = 0;
}

Global::Global()
{
    _setupDefaults();
    _readEnvironment();
}

void Global::_setupDefaults()
{
    // connection
    for( uint32_t i=0; i<ConnectionDescription::IATTR_ALL; ++i )
        _connectionIAttributes[i] = eq::UNDEFINED;
    for( uint32_t i=0; i<ConnectionDescription::CATTR_ALL; ++i )
        _connectionCAttributes[i] = '\0';

    _connectionIAttributes[ConnectionDescription::IATTR_TYPE] = 
        net::CONNECTIONTYPE_TCPIP;
    _connectionIAttributes[ConnectionDescription::IATTR_TCPIP_PORT]     = 0;
    _connectionIAttributes[ConnectionDescription::IATTR_BANDWIDTH]      = 0;
    _connectionIAttributes[ConnectionDescription::IATTR_LAUNCH_TIMEOUT] = 
        60000; // ms
    
    _connectionSAttributes[ConnectionDescription::SATTR_HOSTNAME] = "localhost";
#ifdef WIN32
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c";
    _connectionCAttributes[ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE] =
        '\"';
#else
    _connectionSAttributes[ConnectionDescription::SATTR_LAUNCH_COMMAND] = 
        "ssh -n %h %c >& %h.%n.log";
    _connectionCAttributes[ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE] =
        '\'';
#endif

    // config
    for( uint32_t i=0; i<Config::FATTR_ALL; ++i )
        _configFAttributes[i] = 0.f;

    _configFAttributes[Config::FATTR_EYE_BASE]         = 0.05f;

    // node
    for( uint32_t i=0; i < eq::Node::IATTR_ALL; ++i )
        _nodeIAttributes[i] = eq::UNDEFINED;

#ifdef NDEBUG
    _nodeIAttributes[eq::Node::IATTR_HINT_STATISTICS]   = eq::FASTEST;
#else
    _nodeIAttributes[eq::Node::IATTR_HINT_STATISTICS]   = eq::NICEST;
#endif

    // pipe
    for( uint32_t i=0; i<Pipe::IATTR_ALL; ++i )
        _pipeIAttributes[i] = eq::UNDEFINED;

    _pipeIAttributes[Pipe::IATTR_HINT_THREAD] = eq::ON;

    // window
    for( uint32_t i=0; i<eq::Window::IATTR_ALL; ++i )
        _windowIAttributes[i] = eq::UNDEFINED;

    _windowIAttributes[eq::Window::IATTR_HINT_STEREO]       = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_DOUBLEBUFFER] = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_HINT_FULLSCREEN]   = eq::OFF;
    _windowIAttributes[eq::Window::IATTR_HINT_DECORATION]   = eq::ON;
    _windowIAttributes[eq::Window::IATTR_HINT_DRAWABLE]     = eq::WINDOW;
    _windowIAttributes[eq::Window::IATTR_HINT_SCREENSAVER]  = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_PLANES_COLOR]      = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_PLANES_DEPTH]      = eq::AUTO;
    _windowIAttributes[eq::Window::IATTR_PLANES_STENCIL]    = eq::AUTO;
#ifdef NDEBUG
    _windowIAttributes[eq::Window::IATTR_HINT_STATISTICS]   = eq::FASTEST;
#else
    _windowIAttributes[eq::Window::IATTR_HINT_STATISTICS]   = eq::NICEST;
#endif
    
    // channel
    for( uint32_t i=0; i<eq::Channel::IATTR_ALL; ++i )
        _channelIAttributes[i] = eq::UNDEFINED;

#ifdef NDEBUG
    _channelIAttributes[eq::Channel::IATTR_HINT_STATISTICS] = eq::FASTEST;
#else
    _channelIAttributes[eq::Channel::IATTR_HINT_STATISTICS] = eq::NICEST;
#endif

    // compound
    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
        _compoundIAttributes[i] = eq::UNDEFINED;
}

void Global::_readEnvironment()
{
    for( uint32_t i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const string& name     = ConnectionDescription::getSAttributeString(
            (ConnectionDescription::SAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionSAttributes[i] = envValue;
    }
    for( uint32_t i=0; i<ConnectionDescription::CATTR_ALL; ++i )
    {
        const string& name     = ConnectionDescription::getCAttributeString(
            (ConnectionDescription::CAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionCAttributes[i] = envValue[0];
    }
    for( uint32_t i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const string& name     = ConnectionDescription::getIAttributeString(
            (ConnectionDescription::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<Config::FATTR_ALL; ++i )
    {
        const string& name     = Config::getFAttributeString(
            (Config::FAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _configFAttributes[i] = atof( envValue );
    }

    for( uint32_t i=0; i<eq::Node::IATTR_ALL; ++i )
    {
        const string& name     = eq::Node::getIAttributeString(
            (eq::Node::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _nodeIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<Pipe::IATTR_ALL; ++i )
    {
        const string& name     = Pipe::getIAttributeString(
            (Pipe::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _pipeIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const string& name     = eq::Window::getIAttributeString(
            (eq::Window::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _windowIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<eq::Channel::IATTR_ALL; ++i )
    {
        const string& name     = eq::Channel::getIAttributeString(
            (eq::Channel::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _channelIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
    {
        const string& name     = Compound::getIAttributeString(
            (Compound::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _compoundIAttributes[i] = atol( envValue );
    }
}

#define GLOBAL_ATTR_LENGTH 50

std::ostream& operator << ( std::ostream& os, const Global* global )
{
    Global reference;
    reference._setupDefaults(); // ignore environment variables

    os << disableFlush << disableHeader << "global" << endl;
    os << '{' << indent << endl;

    for( uint32_t i=0; i<ConnectionDescription::IATTR_ALL; ++i )
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
                os << ( value == net::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
                        value == net::CONNECTIONTYPE_SDP   ? "SDP" : 
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

    for( uint32_t i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const string& value = global->_connectionSAttributes[i];
        if( value == reference._connectionSAttributes[i] )
            continue;

        const string& name = ConnectionDescription::getSAttributeString( 
            static_cast<ConnectionDescription::SAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "\"" << value << "\"" << endl;
    }

    for( uint32_t i=0; i<ConnectionDescription::CATTR_ALL; ++i )
    {
        const char value = global->_connectionCAttributes[i];
        if( value == reference._connectionCAttributes[i] )
            continue;

        const string& name = ConnectionDescription::getCAttributeString( 
            static_cast<ConnectionDescription::CAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "'" << value << "'" << endl;
    }

    for( uint32_t i=0; i<Config::FATTR_ALL; ++i )
    {
        const float value = global->_configFAttributes[i];
        if( value == reference._configFAttributes[i] )
            continue;

        const string& name = Config::getFAttributeString( 
            static_cast<Config::FAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << value << endl;
    }

    for( uint32_t i=0; i < eq::Node::IATTR_ALL; ++i )
    {
        const int32_t value = global->_nodeIAttributes[i];
        if( value == reference._nodeIAttributes[i] )
            continue;

        const string& name = eq::Node::getIAttributeString( 
            static_cast<eq::Node::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( uint32_t i=0; i<Pipe::IATTR_ALL; ++i )
    {
        const int value = global->_pipeIAttributes[i];
        if( value == reference._pipeIAttributes[i] )
            continue;

        const string& name = Pipe::getIAttributeString( 
            static_cast<Pipe::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( uint32_t i=0; i<eq::Window::IATTR_ALL; ++i )
    {
        const int value = global->_windowIAttributes[i];
        if( value == reference._windowIAttributes[i] )
            continue;

        const string& name = eq::Window::getIAttributeString( 
            static_cast<eq::Window::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( uint32_t i=0; i<eq::Channel::IATTR_ALL; ++i )
    {
        const int value = global->_channelIAttributes[i];
        if( value == reference._channelIAttributes[i] )
            continue;

        const string& name = eq::Channel::getIAttributeString(
            static_cast<eq::Channel::IAttribute>( i ));
        os << name << string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }

    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
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

}
}
