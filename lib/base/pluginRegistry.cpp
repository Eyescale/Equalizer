
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "debug.h"
#include "dso.h"
#include "file.h"
#include "global.h"
#include "plugin.h"

#include <vector>
#include <typeinfo>

namespace eq
{
namespace base
{

void PluginRegistry::init()
{
    EQASSERT( _plugins.empty( ));

    // add self
    _initPlugin( "" );
    EQASSERT( _plugins.size() == 1 );

    // search all plugin directories for plugin DSOs
    const Strings& directories = Global::getPluginDirectories();

    // for each directory
    for( Strings::const_iterator i = directories.begin();
         i != directories.end(); ++i )
    {
        const std::string& directory = *i;
        EQLOG( LOG_PLUGIN ) << "Searching plugins in " << directory
                            << std::endl;

        // search the number of files in the director
#ifdef WIN32
        Strings files = searchDirectory( directory, "EqualizerCompressor*.dll");
        const char DIRSEP = '\\';
#elif defined (Darwin)
        Strings files = searchDirectory( directory, "libeqCompressor*dylib" );
        const char DIRSEP = '/';
#else
        Strings files = searchDirectory( directory, "libeqCompressor*so" );
        const char DIRSEP = '/';
#endif
        
        // for each file in the directory
        for( Strings::const_iterator j = files.begin(); j != files.end(); ++j )
        {
            // build path + name of library
            const std::string libraryName = 
                directory.empty() ? *j : directory + DIRSEP + *j;
            _initPlugin( libraryName );
        }
    }
}

void PluginRegistry::_initPlugin( const std::string& filename )
{
    Plugin* plugin = new Plugin(); 
    bool add = plugin->init( filename );

    const CompressorInfos& infos = plugin->getInfos();
    if( infos.empty( ))
        add = false;
            
    for( Plugins::const_iterator i = _plugins.begin();
         add && i != _plugins.end(); ++i )
    {
        const CompressorInfos& infos2 = (*i)->getInfos();
        // Simple test to avoid using the same dll twice
        if( infos.front().name == infos2.front().name )
            add = false;
    }

    if( add )
    {
        _plugins.push_back( plugin );
        if( filename.empty( ))
            EQLOG( LOG_PLUGIN ) << "Found " << plugin->getInfos().size()
                                << " built-in compression engines" << std::endl;
        else
            EQLOG( LOG_PLUGIN )
                << "Found " << plugin->getInfos().size()
                << " compression engines in " << filename << std::endl;
    }
    else
        delete plugin;
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
