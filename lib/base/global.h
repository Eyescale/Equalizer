
/* Copyright (c)  2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQBASE_GLOBAL_H
#define EQBASE_GLOBAL_H

#include "base.h"
#include "lock.h" // member
#include "types.h"
#include <string>

namespace eq
{
namespace base
{
    class PluginRegistry;

    /** @cond IGNORE */
    bool testInitPluginDirectories();
    /** @endcond */

    /** 
     * Global parameter handling for the Equalizer base library. 
     */
    class Global
    {
    public:
        /**
          * @return all directories to search for compressor DSOs during
          *         eq::base::init().
          */
        EQ_EXPORT static const Strings& getPluginDirectories();

        /** add a new directory to search for compressor DSOs. */
        EQ_EXPORT static void  addPluginDirectory( const std::string& path );

        /** remove a plugin directory */
        EQ_EXPORT static void  removePluginDirectory( const std::string& path );

        /** @return the plugin registry. */
        EQ_EXPORT static PluginRegistry& getPluginRegistry();

    private:
        EQ_EXPORT friend bool init( const int argc, char** argv );
        EQ_EXPORT friend bool exit();

        static Strings _initPluginDirectories();
        friend bool testInitPluginDirectories();

        static PluginRegistry _pluginRegistry;
        static Strings _pluginDirectories;
    };
}
}

#endif // EQ_GLOBAL_H

