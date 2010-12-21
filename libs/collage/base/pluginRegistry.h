
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQBASE_PLUGINREGISTRY_H
#define EQBASE_PLUGINREGISTRY_H

#include <co/base/os.h>

namespace eq 
{
namespace base
{
    /** The registry for all loaded Equalizer plugins. */
    class PluginRegistry
    {
    public:
        /**
         * @internal
         * Construct a new plugin registry.
         *
         * The plugin registry used by Equalizer can be obtained using
         * Global::getPluginRegistry().
         */
        PluginRegistry();

        /**
         * Add a new directory to search for compressor DSOs during init().
         * @version 1.0
         */
        EQBASE_API void addDirectory( const std::string& path );

        /** Remove a plugin directory. @version 1.0 */
        EQBASE_API void removeDirectory( const std::string& path );

        /**
         * @return all directories to search for compressor DSOs during init().
         * @version 1.0
         */
        EQBASE_API const Strings& getDirectories() const;

        /** @internal Search all plugin directories and register found DSOs */
        void init();
        
        /** @internal Exit all library and free all plugins */
        void exit();
        
        /** @internal @return all registered compressor plugins */
        EQBASE_API const Plugins& getPlugins() const;

        /** @internal @return the plugin containing the given compressor. */
        EQBASE_API Plugin* findPlugin( const uint32_t name );

    private:
        Strings _directories;
        Plugins _plugins;

        /** Initialize a single DSO .*/
        void _initPlugin( const std::string& filename );
    };
}
}
#endif // EQBASE_PLUGINREGISTRY_H
