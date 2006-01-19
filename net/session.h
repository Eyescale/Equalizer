
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include <eq/base/idPool.h>
#include <eq/base/requestHandler.h>

#include "base.h"
#include "commands.h"
#include "global.h"
#include "idHash.h"
#include "node.h"
#include "object.h"

namespace eqNet
{
    /**
     * Manages a session.
     *
     * A session provides unique identifiers for a number of nodes.
     */
    class Session : public Base
    {
    public:
        /** 
         * Constructs a new session.
         *
         * @param nCommands the highest command ID to be handled by the node, at
         *                  least <code>CMD_SESSION_CUSTOM</code>.
         */
        Session( const uint32_t nCommands = CMD_SESSION_CUSTOM );

        /** 
         * Returns the name of the session.
         * 
         * @return the name of the session.
         */
        const std::string& getName() const { return _name; }

        /** 
         * Returns the identifier of this session.
         * 
         * @return the identifier.
         */
        uint32_t getID() const { return _id; }

        /** 
         * Returns the local node holding this session.
         * @return the local node holding this session. 
         */
        Node* getNode(){ return _localNode.get(); }

        /** 
         * Dispatches a command packet to the appropriate object.
         * 
         * @param node the node which sent the packet.
         * @param packet the packet.
         * @sa handleCommand
         */
        void dispatchPacket( Node* node, const Packet* packet );

        /**
         * @name Operations
         */
        //*{
        /** 
         * Generates a continous block of unique identifiers.
         * 
         * @param range the size of the block.
         * @return the first identifier of the block, or <code>0</code> if no
         *         identifier is available.
         */
        uint32_t genIDs( const uint32_t range );

        /** 
         * Frees a continous block of unique identifiers.
         * 
         * @param start the first identifier in the block.
         * @param range the size of the block.
         */
        void freeIDs( const uint32_t start, const uint32_t range );

        /** 
         * Set the master node for a block of identifiers.
         * 
         * This can be used to identify the node which is responsible for the
         * object, action or information associated with an identifier. The
         * identifiers must be unique, it is therefore advised to allocate them
         * using genIDs().
         *
         * The master node must be reachable.
         *
         * @param start the first identifier of the block.
         * @param range the size of the block.
         * @param master the master node for the block of identifiers.
         */
        void setIDMaster( const uint32_t start, const uint32_t range, 
                          Node* master );

        /** 
         * Delete the master node for a block of identifiers.
         * 
         * @param start the first identifier of the block.
         * @param range the size of the block.
         */
        void unsetIDMaster( const uint32_t start, const uint32_t range );

        /** 
         * Returns the master node for an identifier.
         * 
         * @param id the identifier.
         * @return the master node, or an invalid RefPtr if no master node is
         *         set for the identifier.
         */
        Node* getIDMaster( const uint32_t id );

        /** 
         * Registers a new distributed object.
         *
         * The assigned identifier is unique across all registered objects.
         * 
         * @param object the object instance.
         */
        void registerObject( Object* object );
            
        /** 
         * Adds an object using a pre-registered identifier.
         * 
         * @param id the object's unique identifier.
         * @param object the object instance.
         */
        void addRegisteredObject( const uint32_t id, Object* object );

        /** 
         * Returns a registered object.
         * 
         * @param id the object's identifier.
         * @return the registered object.
         */
        Object* getRegisteredObject( const uint32_t id )
            { return _registeredObjects[id]; }

        /** 
         * Deregisters a distributed object.
         * 
         * @param object the object instance.
         */
        void deregisterObject( Object* object );
        //*}
        
    protected:
        /** Registers requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** 
         * Sends a packet to the session's node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet ) { return _server->send( packet ); }

        /** The session's identifier. */
        uint32_t _id;
        
    private:
        friend class Node;
        /** The local node managing the session. */
        eqBase::RefPtr<Node> _localNode;

        /** The node hosting the session. */
        eqBase::RefPtr<Node> _server;

        /** The session's name. */
        std::string _name;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The distributed master identifier pool. */
        eqBase::IDPool _masterPool;

        /** The local identifier pool. */
        eqBase::IDPool _localPool;

        /** Stores a mapping from a block of identifiers to a master node. */
        struct IDMasterInfo
        {
            uint32_t start;
            uint32_t end;
            Node*    master;
        };
        /** The id->master mapping table. */
        std::vector<IDMasterInfo> _idMasterInfos;
        
        /** The registered object, indexed by identifier. */
        IDHash<Object*> _registeredObjects;

        /** The command handler functions. */
        void _cmdGenIDs( Node* node, const Packet* packet );
        void _cmdGenIDsReply( Node* node, const Packet* packet );
        void _cmdSetIDMaster( Node* node, const Packet* packet );
        void _cmdGetIDMaster( Node* node, const Packet* packet );
        void _cmdGetIDMasterReply( Node* node, const Packet* packet );
    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_PRIV_H

