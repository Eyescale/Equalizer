
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_MASTERCM_H
#define EQNET_MASTERCM_H

#include <eq/net/objectCM.h> // base class
#include <eq/net/types.h>       // for Nodes
#include <eq/base/idPool.h>  // ID_ enum
#include <eq/base/mtQueue.h> // member
#include <eq/base/thread.h>  // thread-safety check

namespace eq
{
namespace net
{
    class ObjectDataIStream;

    /** 
     * The base class for versioned master change managers.
     * @internal
     */
    class MasterCM : public ObjectCM
    {
    public:
        MasterCM( Object* object );
        virtual ~MasterCM();

        virtual void makeThreadSafe(){}

        /**
         * @name Versioning
         */
        //@{
        virtual uint32_t commitNB();
        virtual uint32_t commitSync( const uint32_t commitID );

        virtual uint32_t sync( const uint32_t version );

        virtual uint32_t getHeadVersion() const { return _version; }
        virtual uint32_t getVersion() const     { return _version; }
        //@}

        virtual bool isMaster() const { return true; }
        virtual uint32_t getMasterInstanceID() const
            { EQDONTCALL; return EQ_ID_INVALID; }
        virtual void applyMapData() { EQDONTCALL; }
        virtual const Nodes* getSlaveNodes() const { return &_slaves; }

    protected:
        /** The managed object. */
        Object* const _object;

        /** The list of subsribed slave nodes. */
        Nodes _slaves;

        /** The number of object instances subscribed per slave node. */
        base::UUIDHash< uint32_t > _slavesCount;

        /** The current version. */
        uint32_t _version;

        typedef std::pair< base::UUID, ObjectDataIStream* > PendingStream;
        typedef std::vector< PendingStream > PendingStreams;

        /** Not yet ready streams. */
        PendingStreams _pendingDeltas;

        /** The change queue. */
        base::MTQueue< ObjectDataIStream* > _queuedDeltas;

        /* The command handlers. */
        bool _cmdSlaveDelta( Command& command );
        bool _cmdDiscard( Command& command ) { return true; }

        CHECK_THREAD_DECLARE( _cmdThread );
    };
}
}

#endif // EQNET_MASTERCM_H
