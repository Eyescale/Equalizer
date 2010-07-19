
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQADMIN_SERVER_H
#define EQADMIN_SERVER_H

#include <eq/admin/base.h>
#include <eq/admin/types.h>
#include <eq/fabric/server.h>       // base class

namespace eq
{
namespace admin
{
    class NodeFactory;
    class Config;

    /**
     * Proxy object for the connection to an Equalizer server.
     *
     * @sa Client::connectServer
     */
    class Server : public fabric::Server< Client, Server, Config, NodeFactory >
    {
    public:
        /** Construct a new server. @version 1.0 */
        EQADMIN_EXPORT Server();

        /** Destruct the server. @version 1.0 */
        EQADMIN_EXPORT virtual ~Server() {}

        /** Map all server data. @internal */
        void map();
        
        /** Unmap all server data. @internal */
        void unmap();

        virtual void setClient( ClientPtr client ); //!< @internal
        net::CommandQueue* getMainThreadQueue(); //!< @internal

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        bool _cmdMapReply( net::Command& command );
        bool _cmdUnmapReply( net::Command& command );
    };
}
}

#endif // EQADMIN_SERVER_H
