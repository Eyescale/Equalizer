
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

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <eq/base/base.h>
#include <eq/base/lock.h> // member
#include <eq/client/types.h>
#include <string>

bool testInitPluginDirectory();

namespace eq
{
    class NodeFactory;
    class PluginRegistry;
    
    /** Possible values for some integer attributes */
    enum IAttrValue
    {
        UNDEFINED  = -0xfffffff,
        RGBA32F    = -13,
        RGBA16F    = -12,
        FBO        = -11,
        LOCAL_SYNC = -10,
        DRAW_SYNC  = -9,
        ASYNC      = -8,
        PBUFFER    = -7,
        WINDOW     = -6,
        VERTICAL   = -5,
        QUAD       = -4,
        ANAGLYPH   = -3,
        NICEST     = -2,
        AUTO       = -1,
        OFF        = 0,
        ON         = 1,
        FASTEST    = ON,
        HORIZONTAL = ON
    };

    /** 
     * Global parameter handling for the Equalizer client library. 
     */
    class Global
    {
    public:
        /** @return the node factory. */
        static NodeFactory* getNodeFactory() { return _nodeFactory; }

        /** 
         * Set the default Equalizer server.
         * 
         * @param server the default server.
         */
        EQ_EXPORT static void setServer( const std::string& server );

        /** @return the default Equalizer server. */
        EQ_EXPORT static const std::string& getServer();

        /** 
         * Set the config file for the app-local server.
         * 
         * @param configFile the default configFile.
         */
        EQ_EXPORT static void setConfigFile( const std::string& configFile );

        /** @return the default config file for the app-local server. */
        EQ_EXPORT static const std::string& getConfigFile();

        /** 
         * Global lock for all non-thread-safe Carbon API calls. 
         * Note: this is a nop on non-AGL builds. Do not use unless you know the
         * side effects, i.e., ask on the eq-dev mailing list.
         */
        static void enterCarbon();
        /** Global unlock for all non-thread-safe Carbon API calls */
        static void leaveCarbon();

        /**
          * @return all directories to search for compressor DSOs during
          *         eq::init().
          */
        EQ_EXPORT static const StringVector& getPluginDirectories();

        /** add a new directory to search for compressor DSOs. */
        EQ_EXPORT static void  addPluginDirectory( const std::string& path );

        /** remove a plugin directory */
        EQ_EXPORT static void  removePluginDirectory( const std::string& path );

        /** @return the plugin registry. */
        static PluginRegistry* getPluginRegistry() { return _pluginRegistry; }

    private:
        EQ_EXPORT friend bool init( const int argc, char** argv, 
                                    NodeFactory* nodeFactory );
        EQ_EXPORT friend bool exit();

        static StringVector _initPluginDirectory( const char *env );
        friend bool ::testInitPluginDirectory();

        static NodeFactory* _nodeFactory;

        static std::string  _server;
        static std::string  _configFile;
        static PluginRegistry* _pluginRegistry;
        static StringVector _pluginDirectories;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const IAttrValue value );
}

#endif // EQ_GLOBAL_H

