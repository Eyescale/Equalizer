
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/client/api.h>
#include <eq/client/types.h>     // basic typedefs
#include <eq/fabric/server.h>    // base class
#include <co/node.h>             // base class

namespace eq
{
namespace detail { class Server; }

/**
 * Proxy object for the connection to an Equalizer server.
 *
 * The server manages the configurations for Equalizer applications. This proxy
 * object is used to connect to a server and obtain and release a Config from
 * the server.
 *
 * @sa fabric::Server, Client::connectServer()
 */
class Server : public fabric::Server< Client, Server, Config, NodeFactory,
                                      co::Node, ServerVisitor >
{
public:
    /** Construct a new server. @version 1.0 */
    EQ_API Server();

    /** @name Internal */
    //@{
    virtual void setClient( ClientPtr client ); //!< @internal
    EQ_API co::CommandQueue* getMainThreadQueue(); //!< @internal
    EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal
    //@}

    /**
     * Choose a configuration on the server.
     *
     * @param parameters the configuration parameters
     * @return The chosen config, or 0 if no matching config was found.
     * @sa ConfigParams
     * @version 1.0
     */
    EQ_API Config* chooseConfig( const fabric::ConfigParams& parameters );

    /**
     * Release a configuration.
     *
     * The passed configuration will be destroyed by this function and is no
     * longer valid after the call.
     *
     * @param config the configuration.
     * @version 1.0
     */
    EQ_API void releaseConfig( Config* config );

    /** @warning Experimental - may not be supported in the future */
    EQ_API bool shutdown();

protected:
    /** @internal Destruct this server. */
    EQ_API virtual ~Server();

private:
    detail::Server* const _impl;

    /* The command handler functions. */
    bool _cmdChooseConfigReply( co::ICommand& command );
    bool _cmdReleaseConfigReply( co::ICommand& command );
    bool _cmdShutdownReply( co::ICommand& command );
};
}

#endif // EQ_SERVER_H
