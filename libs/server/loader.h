
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_LOADER_H
#define EQSERVER_LOADER_H

#include "api.h"
#include "types.h"

#include <iostream>

namespace eq
{
namespace server
{
    class Channel; 
    class Compound;
    class Config;
    class Node;    
    class Pipe;    
    class Server;
    class Window;  
    
    /**
     * The config file loader.
     */
    class EQSERVER_EXPORT Loader
    {
    public:
        /** 
         * Constructs a new loader.
         */
        Loader() {}

        virtual ~Loader() {}

        /** 
         * Loads a config file.
         * 
         * The returned config has to be deleted by the caller.
         *
         * @param filename the name of the config file.
         * @return The parsed config, or <code>0</code> upon error.
         */
        ServerPtr loadFile( const std::string& filename );

        /** 
         * Parse a config file given as a parameter.
         * 
         * @param config the config file.
         * @return the parsed server.
         */
        ServerPtr parseServer( const char* config );

        /**
         * Add a Compound for each output channel.
         *
         * This function creates a compound for each output channel which is not
         * used as a destination channel yet.
         *
         * @param server the server.
         */
        static void addOutputCompounds( ServerPtr server );

        /**
         * Add segments and layouts for dest channels in non-view configs.
         *
         * This function creates the appropriate views and segments for
         * destination channels, and reassigns the compound channel.
         *
         * @param server the server.
         */
        static void addDestinationViews( ServerPtr server );

        /**
         * Convert config to version 1.1
         *
         * This function converts a 1.0 to a 1.1 configuration.
         * Most notably, the stereo setting is migrated from compounds to
         * views and segments (see
         * <a href="http://www.equalizergraphics.com/documents/design/stereoSwitch.html">Runtime
         * stereo switch doc</a>).
         *
         * @param server the server.
         */
        static void convertTo11( ServerPtr server );

        /**
         * Add one observer for observer-less configurations.
         *
         * If a configuration has no observers, one is created and assigned to
         * all views, which retains the behaviour of legacy configurations.
         *
         * @param server the server.
         */
        static void addDefaultObserver( ServerPtr server );

    private:
        void _parseString( const char* config );
    };
}
}
#endif // EQSERVER_LOADER_H
