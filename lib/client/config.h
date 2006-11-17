
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>
#include <eq/client/commands.h>
#include <eq/net/packets.h>
#include <eq/net/session.h>
#include <eq/client/matrix4.h>

namespace eq
{
    class ConfigEvent;
    class Node;
    class SceneObject;
    class Server;

    class Config : public eqNet::Session
    {
    public:
        /** 
         * Constructs a new config.
         * 
         * @param server the server hosting the config.
         * @param nCommands the highest command ID to be handled by the config, 
         *                  at least <code>CMD_CONFIG_CUSTOM</code>.
         */
        Config();

        virtual ~Config();

        /** 
         * Initialises this configuration.
         * 
         * @param initID an identifier to be passed to all init methods.
         * @return <code>true</code> if the initialisation was successful,
         *         <code>false</code> if not.
         */
        virtual bool init( const uint32_t initID );

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialised. The exit function does not provide the possibility to
         * pass an exit identifier to the exit methods, because individual
         * entities may stopped dynamically by the server when running a config,
         * i.e., before exit() is called.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        virtual bool exit();

        /**
         * @name Frame Control
         */
        //*{
        /** 
         * Requests a new frame of rendering.
         * 
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @return the frame number of the new frame.
         */
        virtual uint32_t beginFrame( const uint32_t frameID );

        /** 
         * Sends frame data to 
         * 
         * @param data 
         */
        void sendSceneObject( SceneObject* object );
        void flushSceneObjects();

        /** 
         * Synchronizes the end of a frame.
         * 
         * @return the frame number of the finished frame, or <code>0</code> if
         *         no frame has been finished.
         */
        virtual uint32_t endFrame();
        //*}

        /** @name Event handling. */
        //*{
        /** 
         * Send an event to the application node.
         * 
         * @param event the event.
         */
        void sendEvent( ConfigEvent& event );

        /** 
         * Get the next received event on the application node.
         * 
         * The returned event is valid until the next call to this method.
         * 
         * @return a config event.
         */
        ConfigEvent* nextEvent();

        /** @return true if events are pending. */
        bool checkEvent() const { return !_eventQueue.empty(); }

        /**
         * Handle all config events.
         *
         * Called at the end of each frame to handle pending config events. The
         * default implementation calls handleEvent() on all pending events,
         * without blocking.
         */
        virtual void handleEvents();

        /** 
         * Handle one config event.
         * 
         * @param event the event.
         * @return <code>true</code> if the event was handled,
         *         <code>false</code> if not.
         */
        virtual bool handleEvent( ConfigEvent* event ){ return false; }
        //*}

        /** Sets the head matrix according to the specified matrix.
         *
         * @param matrix the matrix
         */
        void setHeadMatrix( const vmml::Matrix4f& matrix );

    protected:
        virtual eqNet::Object* instanciateObject( const uint32_t type,
                                    const void* data, const uint64_t dataSize );
    private:
        friend class Server;
        /** The node identifier of the node running the application thread. */
        eqNet::NodeID _appNodeID;
        /** The node running the application thread. */
        eqBase::RefPtr<eqNet::Node> _appNode;

        void _addNode( Node* node );
        void _removeNode( Node* node );

        /** The Matrix for the movement. */
        Matrix4f* _headMatrix;

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The receiver->app thread event queue. */
        eqNet::RequestQueue    _eventQueue;

        /** The command functions. */
        eqNet::CommandResult _cmdCreateNode( eqNet::Node* node,
                                             const eqNet::Packet* packet );
        eqNet::CommandResult _cmdDestroyNode( eqNet::Node* node,
                                              const eqNet::Packet* packet );
        eqNet::CommandResult _cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdBeginFrameReply( eqNet::Node* node, 
                                                  const eqNet::Packet* packet );
        eqNet::CommandResult _cmdEndFrameReply( eqNet::Node* node,
                                                const eqNet::Packet* packet);
        eqNet::CommandResult _cmdEvent( eqNet::Node* node,
                                        const eqNet::Packet* packet );
    };
}

#endif // EQ_CONFIG_H

