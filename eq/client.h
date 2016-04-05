
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

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/api.h>
#include <eq/commandQueue.h> // member
#include <eq/types.h>        // basic types
#include <eq/fabric/client.h>       // base class

namespace eq
{
namespace detail { class Client; }

/**
 * The client represents a network node of the application in the cluster.
 *
 * The methods initLocal() and exitLocal() should be used to set up and exit the
 * listening node instance for each application process.
 *
 * @sa fabric::Client for public methods
 */
class Client : public fabric::Client
{
public:
    /** Construct a new client. @version 1.0 */
    EQ_API Client();

    /**
     * Open and connect an Equalizer server to the local client.
     *
     * The client has to be in the listening state, see initLocal(). Not thread
     * safe.
     *
     * @param server the server.
     * @return true if the server was connected, false if not.
     * @version 1.0
     */
    EQ_API bool connectServer( ServerPtr server );

    /**
     * Disconnect and close the connection to an Equalizer server.
     *
     * Not thread safe.
     *
     * @param server the server.
     * @return true if the server was disconnected, false if not.
     * @version 1.0
     */
    EQ_API bool disconnectServer( ServerPtr server );

    /**
     * Initialize a local, listening node.
     *
     * The following command line options are recognized by this method:
     *
     * <code>--eq-client</code> is used for remote nodes which have been
     * auto-launched by another node, e.g., remote render clients. This method
     * does not return when this command line option is present.
     *
     * <code>--eq-layout</code> can apply multiple times. Each instance has to
     * be followed by the name of a layout. The given layouts will be activated
     * on all canvases using them during Config::init().
     *
     * <code>--eq-modelunit</code> is used for scaling the rendered models in
     * all views. The model unit defines the size of the model wrt the virtual
     * room unit which is always in meter. The default unit is 1 (1 meter or
     * EQ_M).
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @return <code>true</code> if the client was successfully initialized,
     *         <code>false</code> otherwise.
     * @version 1.0
     */
    EQ_API bool initLocal( const int argc, char** argv ) override;

    /** De-initialize a local, listening node. @version 1.1.6 */
    EQ_API bool exitLocal() override;

    /**
     * @return true if the client has commands pending, false otherwise.
     * @version 1.0
     */
    EQ_API bool hasCommands();

    /** @return true if the clientLoop() should keep running. @version 1.11 */
    EQ_API bool isRunning() const;

    /** @internal @return the command queue to the main node thread. */
    EQ_API co::CommandQueue* getMainThreadQueue() override;

    /** Experimental: interrupt main thread queue @internal */
    void interruptMainThread();

    /** @name Data Access */
    //@{
    /** Set the name of the configuration. @version 1.10 */
    EQ_API void setName( const std::string& name );

    /** @return the name of the configuration. @version 1.10 */
    EQ_API const std::string& getName() const;

    /**
     * Add an active layout programmatically, like --eq-layout does.
     * @version 1.9
     */
    EQ_API void addActiveLayout( const std::string& activeLayout );

    /** @internal @return the active layouts given by --eq-layout. */
    const Strings& getActiveLayouts() const;

    /** @internal @return the gpu filter regex given by --eq-gpufilter. */
    const std::string& getGPUFilter() const;

    /** @internal @return the model unit for all views. */
    float getModelUnit() const;
    //@}

protected:
    /** Destruct the client. @version 1.0 */
    EQ_API virtual ~Client();

    /**
     * Implements the processing loop for render clients.
     *
     * As long as the node is running, that is, between initLocal() and an exit
     * send from the server, this method executes received commands using
     * processCommand() and triggers the message pump between commands. This
     * default implementation performs 'while( isRunning( )) processCommand();'.
     *
     * @sa Pipe::createMessagePump(), isRunning(), processCommand()
     * @version 1.0
     */
    EQ_API virtual void clientLoop();

    /** Exit the process cleanly on render clients. @version 1.0 */
    EQ_API virtual void exitClient();

private:
    detail::Client* const _impl;

    /** @sa co::LocalNode::createNode */
    EQ_API co::NodePtr createNode( const uint32_t type ) override;

    /** @internal */
    EQ_API void notifyDisconnect( co::NodePtr node ) override;

    bool _setupClient( const std::string& clientArgs );

    /** The command functions. */
    bool _cmdExit( co::ICommand& command );
    bool _cmdInterrupt( co::ICommand& command );
};
}

#endif // EQ_CLIENT_H
