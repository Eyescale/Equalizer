
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_APPCONFIG_H
#define EQS_APPCONFIG_H

#include "config.h"

#include <eq/packets.h>
#include <eq/net/base.h>

namespace eqs
{
    /**
     * The configuration when allocated by an application.
     */
    class AppConfig : public Config, public eqNet::Base
    {
    public:
        /** 
         * Constructs a new deep copy from a configuration.
         * 
         * @param from the original config.
         */
        AppConfig(const Config& from);

        /** 
         * Sets the identifier of this configuration.
         * 
         * @param id the identifier.
         */
        void setID( const uint id ) { _id = id; }

        /** 
         * Sets the name of the application.
         * 
         * @param name the name of the application.
         */
        void setAppName( const std::string& name )  { _appName = name; }
        
        /** 
         * Sets the name of the render client executable.
         * 
         * @param rc the name of the render client executable.
         */
        void setRenderClient( const std::string rc ){ _renderClient = rc; }

        /** 
         * Handles the received command packet.
         * 
         * @param node the sending node.
         * @param packet the config command packet.
         */
        void handleCommand( eqNet::Node* node,
                            const eq::ConfigPacket* packet );

    private:
        /** The identifier of this configuration. */
        uint _id;

        /** The name of the application. */
        std::string _appName;

        /** The name of the render client executable. */
        std::string _renderClient;

        /** The command handler function table. */
        void (eqs::AppConfig::*_cmdHandler[eq::CMD_CONFIG_ALL])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
    };

    inline std::ostream& operator << ( std::ostream& os,const AppConfig* config)
    {
        if( !config )
        {
            os << "NULL appConfig";
            return os;
        }

        os << "app " << (Config*)config;
        return os;
    }
};
#endif // EQS_APPCONFIG_H
