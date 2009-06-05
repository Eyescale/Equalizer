
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_UNBUFFEREDMASTERCM_H
#define EQNET_UNBUFFEREDMASTERCM_H

#include "objectInstanceDataOStream.h"

#include <eq/net/nodeID.h>             // for NodeIDHash
#include <eq/net/objectCM.h>           // base class
#include <eq/net/types.h>              // for NodeVector

#include <deque>

namespace eq
{
namespace net
{
    class Node;
    class ObjectDeltaDataOStream;

    /** 
     * An object change manager handling versioned objects without any
     * buffering.
     * DeltaSlaveCM is the counterpart for slave instances.
     * @internal
     */
    class UnbufferedMasterCM : public ObjectCM
    {
    public:
        UnbufferedMasterCM( Object* object );
        virtual ~UnbufferedMasterCM();

        virtual void makeThreadSafe(){}

        /**
         * @name Versioning
         */
        //*{
        virtual uint32_t commitNB();
        virtual uint32_t commitSync( const uint32_t commitID );

        virtual void obsolete( const uint32_t version ) { EQUNIMPLEMENTED; }

        virtual void setAutoObsolete( const uint32_t count, 
                                      const uint32_t flags ) { /*nop*/ }
        
        virtual uint32_t getAutoObsoleteCount() const { return 0; }

        virtual uint32_t sync( const uint32_t version )
            { EQDONTCALL; return _version; }

        virtual uint32_t getHeadVersion() const { return _version; }
        virtual uint32_t getVersion() const     { return _version; }
        virtual uint32_t getOldestVersion() const { return _version; }
        //*}

        virtual bool isMaster() const { return true; }
        virtual uint32_t getMasterInstanceID() const
            { EQDONTCALL; return EQ_ID_INVALID; }

        virtual void addSlave( NodePtr node, const uint32_t instanceID,
                               const uint32_t version );
        virtual void removeSlave( NodePtr node );
        virtual void addOldMaster( NodePtr node, const uint32_t instanceID );

        virtual void applyMapData() { EQDONTCALL; }

    private:
        /** The managed object. */
        Object* _object;

        /** The list of subsribed slave nodes. */
        NodeVector _slaves;

        /** The number of object instances subscribed per slave node. */
        NodeIDHash< uint32_t > _slavesCount;

        /** The current head version. */
        uint32_t _version;

        /** The flags for automatic version obsoletion. */
        uint32_t _obsoleteFlags;

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        /* The command handlers. */
        CommandResult _cmdCommit( Command& pkg );
        CommandResult _cmdDiscard( Command& pkg ) { return COMMAND_HANDLED; }

        CHECK_THREAD_DECLARE( _thread );
    };
}

}
#endif // EQNET_UNBUFFEREDMASTERCM_H
