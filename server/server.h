
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_SERVER_H
#define EQSERVER_SERVER_H

#include "base.h"
#include "types.h"
#include "visitorResult.h" // enum

#include <eq/fabric/server.h>    // base class
#include <eq/net/command.h>      // used in inline method
#include <eq/net/commandQueue.h> // member

namespace eq
{
namespace server
{
    /** The Equalizer server. */
    class Server : public fabric::Server< net::Node, Server, Config,
                                          NodeFactory, net::LocalNode >
    {
    public:
        /** 
         * Constructs a new Server.
         */
        EQSERVER_EXPORT Server();

        /** 
         * Initialize the server.
         */
        EQSERVER_EXPORT void init();

        /** 
         * Exit the server.
         */
        EQSERVER_EXPORT void exit();

        /** 
         * The actual main loop of server.
         */
        EQSERVER_EXPORT void handleCommands(); 

        /** 
         * Runs the server. Convenience function for init(), handleCommands() and exit().
         */
        EQSERVER_EXPORT void run();

        /** Delete all configs of this server (exit). */
        EQSERVER_EXPORT void deleteConfigs();

        /** @return the command queue to the server thread */
        net::CommandQueue* getMainThreadQueue() { return &_mainThreadQueue; }

        /** 
         * Traverse this server and all children using a server visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( ServerVisitor& visitor );
        EQSERVER_EXPORT VisitorResult accept( ServerVisitor& visitor ) const;

        /** @return the global time in milliseconds. */
        int64_t getTime() const { return _clock.getTime64(); }

        void registerConfig( Config* config );
        bool deregisterConfig( Config* config );

        /** @sa net::Node::listen() @internal */
        virtual bool listen();

    protected:
        virtual ~Server();

        /** @sa net::Node::dispatchCommand */
        virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand */
        virtual bool invokeCommand( net::Command& command );
        
    private:
        /** The receiver->main command queue. */
        net::CommandQueue _mainThreadQueue;

        /** The global clock. */
        base::Clock _clock;

        net::Nodes _admins; //!< connected admin clients

        /** The current state. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return fabric::NODETYPE_EQ_SERVER; }

        friend class fabric::Config< Server, Config, Observer, Layout, Canvas,
                                     server::Node, ConfigVisitor >;

        /**  Add a new config to this server. */
        void _addConfig( Config* config );

        /** Remove a config from this server. */
        bool _removeConfig( Config* config );        

        /** The command functions. */
        bool _cmdChooseConfig( net::Command& command );
        bool _cmdReleaseConfig( net::Command& command );
        bool _cmdDestroyConfigReply( net::Command& command );
        bool _cmdShutdown( net::Command& command );
        bool _cmdMap( net::Command& command );
        bool _cmdUnmap( net::Command& command );
    };
}
}
#endif // EQSERVER_SERVER_H
