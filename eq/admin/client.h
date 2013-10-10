
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQADMIN_CLIENT_H
#define EQADMIN_CLIENT_H

#include <eq/admin/api.h>
#include <eq/admin/types.h>
#include <eq/fabric/client.h>      // base class
#include <co/commandQueue.h>   // member

namespace eq
{
namespace admin
{
class Server;

/**
 * The client represents the admin tool as a co::Node to the cluster.
 *
 * The methods initLocal() and exitLocal() should be used to set up and exit the
 * listening node instance for each application process.
 */
class Client : public fabric::Client
{
public:
    /** Construct a new client. @version 1.0 */
    EQADMIN_API Client();

    /** Destruct the client. @version 1.0 */
    EQADMIN_API virtual ~Client();

    /**
     * Open and connect an Equalizer server to the local client.
     *
     * The client has to be in the listening state, see initLocal().
     *
     * @param server the server.
     * @return true if the server was connected, false if not.
     * @version 1.0
     */
    EQADMIN_API bool connectServer( ServerPtr server );

    /**
     * Disconnect and close the connection to an Equalizer server.
     *
     * @param server the server.
     * @return true if the server was disconnected, false if not.
     * @version 1.0
     */
    EQADMIN_API bool disconnectServer( ServerPtr server );

    /** @return the command queue to the main node thread. @internal */
    virtual co::CommandQueue* getMainThreadQueue()
        { return &_mainThreadQueue; }

private:
    /** The command->node command queue. */
    co::CommandQueue _mainThreadQueue;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    /** @sa co::LocalNode::createNode */
    EQADMIN_API virtual co::NodePtr createNode( const uint32_t type );
};
}
}

#endif // EQADMIN_CLIENT_H
