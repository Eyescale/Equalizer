
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "base.h"
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
         * Parse a Config given as a parameter.
         * 
         * @param config the config.
         * @return the parsed config.
         */
        Config* parseConfig( const char* config );

        /**
         * Add a Compound for each output channel.
         *
         * This function creates a compound for each output channel which is not
         * used as a destination channel yet.
         */
        static void addOutputCompounds( ServerPtr server );

        /**
         * Add a Segment and Layout for each destination channel.
         *
         * This function creates the appropriate views and segments for
         * destination channels, and reassigns the compound channel.
         */
        static void addDestinationViews( ServerPtr server );

        /**
         * Add one observer for observer-less configurations.
         *
         * If a configuration has no observers, one is created and assigned to
         * all views, which retains the behaviour of legacy configurations.
         */
        static void addDefaultObserver( ServerPtr server );

    private:
        void _parseString( const char* config );
    };
}
}
#endif // EQSERVER_LOADER_H
