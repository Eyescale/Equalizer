
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/net/node.h>

#include "commands.h"

namespace eq
{
    class Config;
    class ConfigParams;
    class Node;
    struct ServerPacket;

    class EQ_EXPORT Server : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new server.
         */
        Server();

        /** 
         * Chooses a configuration on the server.
         * 
         * @param parameters the configuration parameters
         * @return The choosen config, or <code>NULL</code> if no matching
         *         config was found.
         * @sa ConfigParams
         */
        Config* chooseConfig( const ConfigParams& parameters );

        /** 
         * Releases the configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        void    releaseConfig( Config* config );

    protected:
        /**
         * Destructs this server.
         */
        virtual ~Server();

    private:
        /* The command handler functions. */
        eqNet::CommandResult _cmdCreateConfig( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyConfig( eqNet::Command& command );
        eqNet::CommandResult _cmdChooseConfigReply( eqNet::Command& command );
    };

    inline std::ostream& operator << ( std::ostream& os, const Server* server )
    {
        if( !server )
        {
            os << "NULL server";
            return os;
        }

        os << "server " << (void*)server;
        return os;
    }
}

#endif // EQ_SERVER_H

