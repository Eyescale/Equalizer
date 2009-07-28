
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "nodeFactory.h"
#include "pluginRegistry.h"

#include <eq/base/fileSearch.h>
#include <eq/net/global.h>

#include <algorithm>

#ifdef WIN32_API
#  include <direct.h>
#  define getcwd _getcwd
#  ifndef MAXPATHLEN
#    define MAXPATHLEN 1024
#  endif
#endif


namespace eq
{
NodeFactory* Global::_nodeFactory = 0;
PluginRegistry Global::_pluginRegistry;
std::string Global::_server;
std::string Global::_configFile = "examples/configs/4-window.all.eqc";
StringVector Global::_pluginDirectories = _initPluginDirectories();

#ifdef AGL
static base::Lock _carbonLock;
#endif


void Global::setServer( const std::string& server )
{
    _server = server;
}

const std::string& Global::getServer()
{
    return _server;
}

void Global::setConfigFile( const std::string& configFile )
{
    _configFile = configFile;
}

const std::string& Global::getConfigFile()
{
    return _configFile;
}

void Global::enterCarbon()
{
#ifdef AGL
    _carbonLock.set();
#endif
}

void Global::leaveCarbon()
{ 
#ifdef AGL
    _carbonLock.unset();
#endif
}

// initialized by EQ_PLUGIN_PATH:
EQ_EXPORT const StringVector& Global::getPluginDirectories()
{
    return _pluginDirectories;
}

void  Global::addPluginDirectory( const std::string& path )
{
    _pluginDirectories.push_back( path );
}

void  Global::removePluginDirectory( const std::string& path )
{

    StringVector::iterator i = find(_pluginDirectories.begin(), _pluginDirectories.end(), path);

    if( i != _pluginDirectories.end( ))
        _pluginDirectories.erase( i );
}


StringVector Global::_initPluginDirectories()
{
    StringVector pluginDirectories;

    char* env = getenv("EQ_PLUGIN_PATH") ;
    std::string envString( env ? env : "" );

    if( envString.empty( ))
    {
        pluginDirectories.push_back( "/usr/local/share/Equalizer/plugins" );
        pluginDirectories.push_back( ".eqPlugins" );

        char cwd[MAXPATHLEN];
        getcwd( cwd, MAXPATHLEN );
        pluginDirectories.push_back( cwd );

#ifdef WIN32
        if( GetModuleFileName( 0, cwd, MAXPATHLEN ) > 0 )
        {
            pluginDirectories.push_back( base::getDirname( cwd ));
        }
#endif

#ifdef Darwin
        env = getenv( "DYLD_LIBRARY_PATH" );
#else
        env = getenv( "LD_LIBRARY_PATH" );
#endif
        if( env )
            envString = env;
    }

#ifdef WIN32
    const char separator = ';';
#else
    const char separator = ':';
#endif
        
    while( !envString.empty( ))
    {
        size_t nextPos = envString.find( separator );
        if ( nextPos == std::string::npos )
            nextPos = envString.size();

        std::string path = envString.substr( 0, nextPos );
        if ( nextPos == envString.size())
            envString = "";
        else
            envString = envString.substr( nextPos + 1, envString.size() );

        if( !path.empty( ))
            pluginDirectories.push_back( path );
    }

    return pluginDirectories;
}

PluginRegistry& Global::getPluginRegistry()
{
    return _pluginRegistry;
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const IAttrValue value )
{
    if( value > ON ) // ugh
        os << static_cast<int>( value );
    else
        os << ( value == UNDEFINED  ? "UNDEFINED" :
                value == OFF        ? "OFF" :
                value == ON         ? "ON" : 
                value == AUTO       ? "AUTO" :
                value == NICEST     ? "NICEST" :
                value == QUAD       ? "QUAD" :
                value == ANAGLYPH   ? "ANAGLYPH" :
                value == VERTICAL   ? "VERTICAL" :
                value == WINDOW     ? "window" :
                value == PBUFFER    ? "pbuffer" : 
                value == FBO        ? "FBO" : 
                value == RGBA16F    ? "RGBA16F" :
                value == RGBA32F    ? "RGBA32F" :
                value == ASYNC      ? "ASYNC" : 
                value == DRAW_SYNC  ? "DRAW_SYNC" : 
                value == LOCAL_SYNC ? "LOCAL_SYNC" : 
                "ERROR"  );
    return os;
}
}
