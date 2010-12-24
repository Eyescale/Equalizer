
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

#ifndef CO_MASTERCM_H
#define CO_MASTERCM_H

#include <co/objectCM.h> // base class
#include <co/types.h>

#include <co/base/mtQueue.h> // member
#include <co/base/pool.h>    // member
#include <co/base/thread.h>  // thread-safety check

namespace co
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

        virtual void init(){}

        /**
         * @name Versioning
         */
        //@{
        virtual uint32_t commitNB();
        virtual uint128_t commitSync( const uint32_t commitID );

        virtual uint128_t sync( const uint128_t& version );

        virtual uint128_t getHeadVersion() const { return _version; }
        virtual uint128_t getVersion() const     { return _version; }
        //@}

        virtual bool isMaster() const { return true; }
        virtual uint32_t getMasterInstanceID() const
            { EQDONTCALL; return EQ_INSTANCE_INVALID; }
        virtual void applyMapData( const uint128_t& version ) { EQDONTCALL; }
        virtual const Nodes* getSlaveNodes() const { return &_slaves; }

        virtual const Object* getObject( ) const { return _object; }

    protected:
        /** The managed object. */
        Object* _object;

        /** The list of subsribed slave nodes. */
        Nodes _slaves;

        /** The number of object instances subscribed per slave node. */
        stde::hash_map< uint128_t, uint32_t > _slavesCount;

        /** The current version. */
        uint128_t _version;

        typedef std::pair< co::base::UUID, ObjectDataIStream* > PendingStream;
        typedef std::vector< PendingStream > PendingStreams;

        /** Not yet ready streams. */
        PendingStreams _pendingDeltas;

        /** The change queue. */
        co::base::MTQueue< ObjectDataIStream* > _queuedDeltas;

        /** Cached input streams (+decompressor) */
        co::base::Pool< ObjectDataIStream, true > _iStreamCache;

        void _apply( ObjectDataIStream* is );

        /* The command handlers. */
        bool _cmdSlaveDelta( Command& command );
        bool _cmdDiscard( Command& ) { return true; }

        EQ_TS_VAR( _cmdThread );
    };
}

#endif // CO_MASTERCM_H
