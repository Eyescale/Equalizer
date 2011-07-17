
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
        _connectionIAttributes[i] = fabric::UNDEFINED;

    _connectionIAttributes[ ConnectionDescription::IATTR_TYPE ] = 
        co::CONNECTIONTYPE_TCPIP;
    _connectionIAttributes[ ConnectionDescription::IATTR_PORT ] = 0;
    _connectionIAttributes[ ConnectionDescription::IATTR_BANDWIDTH ] = 0;
    _connectionSAttributes[ ConnectionDescription::SATTR_FILENAME ] = "default";

    // config
    for( uint32_t i=0; i<Config::FATTR_ALL; ++i )
        _configFAttributes[i] = 0.f;
    for( uint32_t i=0; i<Config::IATTR_ALL; ++i )
        _configIAttributes[i] = fabric::UNDEFINED;

    _configFAttributes[Config::FATTR_EYE_BASE]         = 0.05f;
    _configIAttributes[Config::IATTR_ROBUSTNESS]       = fabric::AUTO;

    // node
    for( uint32_t i=0; i < Node::CATTR_ALL; ++i )
        _nodeCAttributes[i] = 0;
    for( uint32_t i=0; i < Node::IATTR_ALL; ++i )
        _nodeIAttributes[i] = fabric::UNDEFINED;

    _nodeIAttributes[Node::IATTR_LAUNCH_TIMEOUT] = 60000; // ms
#ifdef WIN32
    _nodeSAttributes[Node::SATTR_LAUNCH_COMMAND] = "ssh -n %h %c";
    _nodeCAttributes[Node::CATTR_LAUNCH_COMMAND_QUOTE] = '\"';
#else
    _nodeSAttributes[Node::SATTR_LAUNCH_COMMAND] = "ssh -n %h %c >& %h.%n.log";
    _nodeCAttributes[Node::CATTR_LAUNCH_COMMAND_QUOTE] = '\'';
#endif

    // pipe
    for( uint32_t i=0; i<Pipe::IATTR_ALL; ++i )
        _pipeIAttributes[i] = fabric::UNDEFINED;

    _pipeIAttributes[Pipe::IATTR_HINT_THREAD] = fabric::ON;
    _pipeIAttributes[Pipe::IATTR_HINT_CUDA_GL_INTEROP] = fabric::OFF;

    // window
    for( uint32_t i=0; i<Window::IATTR_ALL; ++i )
        _windowIAttributes[i] = fabric::UNDEFINED;

    _windowIAttributes[Window::IATTR_HINT_STEREO]       = fabric::AUTO;
    _windowIAttributes[Window::IATTR_HINT_DOUBLEBUFFER] = fabric::AUTO;
    _windowIAttributes[Window::IATTR_HINT_FULLSCREEN]   = fabric::OFF;
    _windowIAttributes[Window::IATTR_HINT_DECORATION]   = fabric::ON;
    _windowIAttributes[Window::IATTR_HINT_DRAWABLE]     = fabric::WINDOW;
    _windowIAttributes[Window::IATTR_HINT_SCREENSAVER]  = fabric::AUTO;
    _windowIAttributes[Window::IATTR_PLANES_COLOR]      = fabric::AUTO;
    _windowIAttributes[Window::IATTR_PLANES_DEPTH]      = fabric::AUTO;
    _windowIAttributes[Window::IATTR_PLANES_STENCIL]    = fabric::AUTO;
#ifdef NDEBUG
    _windowIAttributes[Window::IATTR_HINT_STATISTICS]   = fabric::FASTEST;
#else
    _windowIAttributes[Window::IATTR_HINT_STATISTICS]   = fabric::NICEST;
#endif
    
    // channel
    for( uint32_t i=0; i<Channel::IATTR_ALL; ++i )
        _channelIAttributes[i] = fabric::UNDEFINED;

#ifdef NDEBUG
    _channelIAttributes[Channel::IATTR_HINT_STATISTICS] = fabric::FASTEST;
#else
    _channelIAttributes[Channel::IATTR_HINT_STATISTICS] = fabric::NICEST;
#endif
    _channelIAttributes[Channel::IATTR_HINT_SENDTOKEN] = fabric::OFF;

    // compound
    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
        _compoundIAttributes[i] = fabric::UNDEFINED;
}

void Global::_readEnvironment()
{
    for( uint32_t i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const std::string& name = ConnectionDescription::getSAttributeString(
            (ConnectionDescription::SAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionSAttributes[i] = envValue;
    }
    for( uint32_t i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const std::string& name = ConnectionDescription::getIAttributeString(
            (ConnectionDescription::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _connectionIAttributes[i] = atol( envValue );
    }

    for( uint32_t i=0; i < Config::FATTR_LAST; ++i )
    {
        const std::string& name = Config::getFAttributeString(
            (Config::FAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _configFAttributes[i] = atof( envValue );
    }
    for( uint32_t i=0; i < Config::IATTR_LAST; ++i )
    {
        const std::string& name = Config::getIAttributeString(
            (Config::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _configIAttributes[i] = atol( envValue );
    }

    for( uint32_t i=0; i < Node::SATTR_LAST; ++i )
    {
        const std::string& name = Node::getSAttributeString(
            (Node::SAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _nodeSAttributes[i] = envValue;
    }
    for( uint32_t i=0; i < Node::CATTR_LAST; ++i )
    {
        const std::string& name = Node::getCAttributeString(
            (Node::CAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _nodeCAttributes[i] = envValue[0];
    }
    for( uint32_t i=0; i < Node::IATTR_LAST; ++i )
    {
        const std::string& name     = Node::getIAttributeString(
            (Node::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _nodeIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<Pipe::IATTR_LAST; ++i )
    {
        const std::string& name     = Pipe::getIAttributeString(
            (Pipe::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _pipeIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i < Window::IATTR_LAST; ++i )
    {
        const std::string& name     = Window::getIAttributeString(
            (Window::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _windowIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i < Channel::IATTR_LAST; ++i )
    {
        const std::string& name = Channel::getIAttributeString(
            (Channel::IAttribute)i);
        const char*   envValue = getenv( name.c_str( ));
        
        if( envValue )
            _channelIAttributes[i] = atol( envValue );
    }
    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
    {
        const std::string& name     = Compound::getIAttributeString(
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

    os << co::base::disableFlush << co::base::disableHeader
       << "#Equalizer " << global->getConfigFAttribute( Config::FATTR_VERSION )
       << " ascii" << std::endl << std::endl
       << "global" << std::endl
       << '{' << co::base::indent << std::endl;

    for( uint32_t i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const int value = global->_connectionIAttributes[i];
        if( value == reference._connectionIAttributes[i] )
            continue;

        const std::string& name = ConnectionDescription::getIAttributeString( 
            static_cast<ConnectionDescription::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' );
                
        switch( i )
        { 
            case ConnectionDescription::IATTR_TYPE:
                os << static_cast< co::ConnectionType >( value );
                break;
            default:
                os << value;
        }
        os << std::endl;
    }

    for( uint32_t i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const std::string& value = global->_connectionSAttributes[i];
        if( value == reference._connectionSAttributes[i] )
            continue;

        const std::string& name = ConnectionDescription::getSAttributeString( 
            static_cast<ConnectionDescription::SAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "\"" << value << "\"" << std::endl;
    }

    for( uint32_t i=0; i<Config::FATTR_ALL; ++i )
    {
        if( i == Config::FATTR_VERSION )
            continue;

        const float value = global->_configFAttributes[i];
        if( value == reference._configFAttributes[i] )
            continue;

        const std::string& name = Config::getFAttributeString( 
            static_cast<Config::FAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << value << std::endl;
    }

    for( uint32_t i=0; i<Config::IATTR_ALL; ++i )
    {
        const int32_t value = global->_configIAttributes[i];
        if( value == reference._configIAttributes[i] )
            continue;

        const std::string& name = Config::getIAttributeString( 
            static_cast<Config::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    for( uint32_t i=0; i<Node::SATTR_ALL; ++i )
    {
        const std::string& value = global->_nodeSAttributes[i];
        if( value == reference._nodeSAttributes[i] )
            continue;

        const std::string& name = Node::getSAttributeString( 
            static_cast<Node::SAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "\"" << value << "\"" << std::endl;
    }

    for( uint32_t i=0; i<Node::CATTR_ALL; ++i )
    {
        const char value = global->_nodeCAttributes[i];
        if( value == reference._nodeCAttributes[i] )
            continue;

        const std::string& name = Node::getCAttributeString( 
            static_cast<Node::CAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << "'" << value << "'" << std::endl;
    }

    for( uint32_t i=0; i < Node::IATTR_ALL; ++i )
    {
        const int32_t value = global->_nodeIAttributes[i];
        if( value == reference._nodeIAttributes[i] )
            continue;

        const std::string& name = Node::getIAttributeString( 
            static_cast<Node::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    for( uint32_t i=0; i<Pipe::IATTR_ALL; ++i )
    {
        const int value = global->_pipeIAttributes[i];
        if( value == reference._pipeIAttributes[i] )
            continue;

        const std::string& name = Pipe::getIAttributeString( 
            static_cast<Pipe::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    for( uint32_t i=0; i<Window::IATTR_ALL; ++i )
    {
        const int value = global->_windowIAttributes[i];
        if( value == reference._windowIAttributes[i] )
            continue;

        const std::string& name = Window::getIAttributeString( 
            static_cast<Window::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    for( uint32_t i=0; i<Channel::IATTR_ALL; ++i )
    {
        const int value = global->_channelIAttributes[i];
        if( value == reference._channelIAttributes[i] )
            continue;

        const std::string& name = Channel::getIAttributeString(
            static_cast<Channel::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    for( uint32_t i=0; i<Compound::IATTR_ALL; ++i )
    {
        const int value = global->_compoundIAttributes[i];
        if( value == reference._compoundIAttributes[i] )
            continue;

        const std::string& name = Compound::getIAttributeString(
            static_cast<Compound::IAttribute>( i ));
        os << name << std::string( GLOBAL_ATTR_LENGTH - name.length(), ' ' );

        switch( i )
        {
            case Compound::IATTR_STEREO_MODE:
                os << static_cast< fabric::IAttribute >( value ) << std::endl;
                break;

            case Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK:
            case Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK:
                os << ColorMask( value ) << std::endl;
                break;

            default:
                EQASSERTINFO( 0, "unimplemented" );
        }
    }

    os << co::base::exdent << '}' << std::endl
       << co::base::enableHeader << co::base::enableFlush;
    return os;
}

}
}
