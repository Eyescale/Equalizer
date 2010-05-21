
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "file.h"
#include "global.h"
#include "pluginRegistry.h"

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
namespace base
{

PluginRegistry Global::_pluginRegistry;
Strings Global::_pluginDirectories = _initPluginDirectories();

// initialized by EQ_PLUGIN_PATH:
EQ_EXPORT const Strings& Global::getPluginDirectories()
{
    return _pluginDirectories;
}

void  Global::addPluginDirectory( const std::string& path )
{
    _pluginDirectories.push_back( path );
}

void  Global::removePluginDirectory( const std::string& path )
{

    Strings::iterator i = find( _pluginDirectories.begin(),
                                _pluginDirectories.end(), path );

    if( i != _pluginDirectories.end( ))
        _pluginDirectories.erase( i );
}


Strings Global::_initPluginDirectories()
{
    Strings pluginDirectories;

    char* env = getenv( "EQ_PLUGIN_PATH" );
    std::string envString( env ? env : "" );

    if( envString.empty( ))
    {
        pluginDirectories.push_back( "/usr/local/share/Equalizer/plugins" );
        pluginDirectories.push_back( ".eqPlugins" );

        char cwd[MAXPATHLEN];
        pluginDirectories.push_back( getcwd( cwd, MAXPATHLEN ));

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

}
}
