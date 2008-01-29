
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/client/client.h>    // called in inline method
#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum
#include <eq/net/node.h>         // base class

namespace eq
{
    class Client;
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

        /** @name Data Access */
        //*{
        eqBase::RefPtr<Client> getClient(){ return _client; }
        //*}

        /** 
         * Chooses a configuration on the server.
         * 
         * @param parameters the configuration parameters
         * @return The chosen config, or <code>NULL</code> if no matching
         *         config was found.
         * @sa ConfigParams
         */
        Config* chooseConfig( const ConfigParams& parameters );

        /** Undocumented - may not be supported in the future */
        Config* useConfig( const ConfigParams& parameters, 
                           const std::string& config );

        /** 
         * Releases the configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        void releaseConfig( Config* config );

        /** Undocumented - may not be supported in the future */
        bool shutdown();

    protected:
        /**
         * Destructs this server.
         */
        virtual ~Server();

    private:
        /** The local client connected to the server */
        eqBase::RefPtr<Client> _client;
        friend class Client;

        /** @sa eqNet::Node::getType */
        virtual uint32_t getType() const { return TYPE_EQ_SERVER; }

        /* The command handler functions. */
        eqNet::CommandResult _reqCreateConfig( eqNet::Command& command );
        eqNet::CommandResult _reqDestroyConfig( eqNet::Command& command );
        eqNet::CommandResult _reqChooseConfigReply( eqNet::Command& command );
        eqNet::CommandResult _reqReleaseConfigReply( eqNet::Command& command );
        eqNet::CommandResult _cmdShutdownReply( eqNet::Command& command );
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

