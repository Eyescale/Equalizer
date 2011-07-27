
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "pluginRegistry.h"

#include "compressorInfo.h"
#include "debug.h"
#include "dso.h"
#include "file.h"
#include "global.h"
#include "os.h"
#include "plugin.h"
#include "stdExt.h"

#include <vector>
#include <typeinfo>

#ifdef _MSC_VER
#  include <direct.h>
#  define getcwd _getcwd
#else
#  include <unistd.h>   // for getcwd
#endif

#ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
#endif

namespace co
{
namespace base
{
namespace
{
Strings _initPluginDirectories()
{
    Strings pluginDirectories;

    char* env = getenv( "EQ_PLUGIN_PATH" );
    std::string envString( env ? env : "" );

    if( envString.empty( ))
    {
        char cwd[MAXPATHLEN];
        pluginDirectories.push_back( getcwd( cwd, MAXPATHLEN ));

#ifdef _WIN32
        if( GetModuleFileName( 0, cwd, MAXPATHLEN ) > 0 )
            pluginDirectories.push_back( getDirname( cwd ));
#endif

#ifdef Darwin
        env = getenv( "DYLD_LIBRARY_PATH" );
#else
        env = getenv( "LD_LIBRARY_PATH" );
#endif
        if( env )
            envString = env;
    }

#ifdef _WIN32
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

}

PluginRegistry::PluginRegistry()
        : _directories( _initPluginDirectories( ))
{}
 
const Strings& PluginRegistry::getDirectories() const
{
    return _directories;
}

void  PluginRegistry::addDirectory( const std::string& path )
{
    _directories.push_back( path );
}

void PluginRegistry::removeDirectory( const std::string& path )
{
    Strings::iterator i = stde::find( _directories, path );
    if( i != _directories.end( ))
        _directories.erase( i );
}

void PluginRegistry::init()
{
    // for each directory
    for( Strings::const_iterator i = _directories.begin();
         i != _directories.end(); ++i )
    {
        const std::string& dir = *i;
        EQLOG( LOG_PLUGIN ) << "Searching plugins in " << dir << std::endl;

#ifdef _WIN32
        Strings files = searchDirectory( dir, "EqualizerCompressor*.dll");
        const char DIRSEP = '\\';
#elif defined (Darwin)
        Strings files = searchDirectory( dir, "libEqualizerCompressor*.dylib" );
        Strings oldFiles = searchDirectory( dir, "libeqCompressor*.dylib" );
        files.insert( files.end(), oldFiles.begin(), oldFiles.end( ));
        const char DIRSEP = '/';
#else
        Strings files = searchDirectory( dir, "libEqualizerCompressor*.so" );
        Strings oldFiles = searchDirectory( dir, "libeqCompressor*.so" );
        files.insert( files.end(), oldFiles.begin(), oldFiles.end( ));
        const char DIRSEP = '/';
#endif
        
        // for each file found in the directory
        for( Strings::const_iterator j = files.begin(); j != files.end(); ++j )
        {
            // build path + name of library
            const std::string libraryName =
                dir.empty() ? *j : dir + DIRSEP + *j;
            addPlugin( libraryName );
        }
    }

    for( Plugins::const_iterator i = _plugins.begin(); i != _plugins.end(); ++i)
    {
        Plugin* plugin = *i;
        plugin->initChildren();
    }
}

bool PluginRegistry::addPlugin( const std::string& filename )
{
    Plugin* plugin = new Plugin(); 
    if( !plugin->init( filename ))
    {
        delete plugin;
        return false;
    }

    const CompressorInfos& infos = plugin->getInfos();
    for( Plugins::const_iterator i = _plugins.begin(); i != _plugins.end(); ++i)
    {
        const CompressorInfos& infos2 = (*i)->getInfos();

        // Simple test to avoid loading the same dll twice
        if( infos.front().name == infos2.front().name )
        {
            delete plugin;
            return true;
        }
    }

    _plugins.push_back( plugin );
    EQLOG( LOG_PLUGIN ) << "Found " << plugin->getInfos().size()
                        << " compression engines in " << filename << std::endl;
    return true;
}

void PluginRegistry::exit()
{
    for( Plugins::const_iterator i = _plugins.begin(); i != _plugins.end(); ++i)
    {
        Plugin* plugin = *i;
        plugin->exit();
        delete plugin;
    }

    _plugins.clear();
}

Plugin* PluginRegistry::findPlugin( const uint32_t name )
{

    for( Plugins::const_iterator i = _plugins.begin(); i != _plugins.end(); ++i)
    {
        Plugin* plugin = *i;
        if ( plugin->implementsType( name ))
            return plugin;
    }

    return 0;
}

const Plugins& PluginRegistry::getPlugins() const
{
    return _plugins;
}

}
}
