
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
         * Parse a config given as a parameter.
         * 
         * @param config the config.
         * @return the parsed config.
         */
        Config* parseConfig( const char* config );

        /**
         * Add a compound for each output channel.
         *
         * This function creates a compound for each output channel which is not
         * used as a destination channel yet.
         */
        static void addOutputCompounds( ServerPtr server );

    private:
        void _parseString( const char* config );
    };
}
}
#endif // EQSERVER_LOADER_H
