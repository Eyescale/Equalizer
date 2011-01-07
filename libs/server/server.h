
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

#include "api.h"
#include "types.h"
#include "visitorResult.h" // enum

#include <eq/fabric/server.h>    // base class
#include <co/command.h>      // used in inline method
#include <co/commandQueue.h> // member

namespace eq
{
namespace server
{
    /** The Equalizer server. */
    class Server : public fabric::Server< co::Node, Server, Config,
                                          NodeFactory, co::LocalNode >
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
        co::CommandQueue* getMainThreadQueue() { return &_mainThreadQueue; }

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

        /** @sa co::Node::listen() @internal */
        virtual bool listen();

    protected:
        virtual ~Server();

        /** @sa co::Node::dispatchCommand */
        virtual bool dispatchCommand( co::Command& command );
        
    private:
        /** The receiver->main command queue. */
        co::CommandQueue _mainThreadQueue;

        /** The global clock. */
        co::base::Clock _clock;

        co::Nodes _admins; //!< connected admin clients

        /** The current state. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa co::Node::getType */
        virtual uint32_t getType() const { return fabric::NODETYPE_EQ_SERVER; }

        friend class fabric::Config< Server, Config, Observer, Layout, Canvas,
                                     server::Node, ConfigVisitor >;

        /**  Add a new config to this server. */
        void _addConfig( Config* config );

        /** Remove a config from this server. */
        bool _removeConfig( Config* config );        

        /** The command functions. */
        bool _cmdChooseConfig( co::Command& command );
        bool _cmdReleaseConfig( co::Command& command );
        bool _cmdDestroyConfigReply( co::Command& command );
        bool _cmdShutdown( co::Command& command );
        bool _cmdMap( co::Command& command );
        bool _cmdUnmap( co::Command& command );
    };
}
}
#endif // EQSERVER_SERVER_H
