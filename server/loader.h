
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_H
#define EQS_LOADER_H

#include <iostream>

namespace eqs
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
    class Loader
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
         * @return The parsed config, or <code>NULL</code> upon error.
         */
        Server* loadConfig( const std::string& filename );

        /**
         * @name Factory Methods.
         */
        //*{
        virtual Server*   createServer();
        virtual Config*   createConfig();
        virtual Node*     createNode();
        virtual Pipe*     createPipe();
        virtual Window*   createWindow();
        virtual Channel*  createChannel();
        virtual Compound* createCompound();
        //*}

    private:
    };
};

#endif // EQS_LOADER_H
