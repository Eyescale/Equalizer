
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_VERSIONEDSLAVECM_H
#define CO_VERSIONEDSLAVECM_H

#include "objectCM.h"            // base class
#include "objectDataIStream.h"      // member
#include "objectSlaveDataOStream.h" // member

#include <lunchbox/mtQueue.h>     // member
#include <lunchbox/pool.h>        // member
#include <lunchbox/thread.h>      // thread-safety macro

namespace co
{
    class Node;

    /** 
     * An object change manager handling changes for versioned slave instances.
     */
    class VersionedSlaveCM : public ObjectCM
    {
    public:
        VersionedSlaveCM( Object* object, uint32_t masterInstanceID );
        virtual ~VersionedSlaveCM();

        virtual void init() {}

        /**
         * @name Versioning
         */
        //@{
        virtual uint128_t commit( const uint32_t incarnation );
        virtual uint128_t sync( const uint128_t& version );

        virtual uint128_t getHeadVersion() const;
        virtual uint128_t getVersion() const { return _version; }
        //@}

        virtual bool isMaster() const { return false; }

        virtual uint32_t getMasterInstanceID() const {return _masterInstanceID;}
        virtual void setMasterNode( NodePtr node ) { _master = node; }
        virtual NodePtr getMasterNode() { return _master; }

        virtual void addSlave( Command& command ) { EQDONTCALL; }
        virtual void removeSlaves( NodePtr ) {}

        virtual void applyMapData( const uint128_t& version );
        virtual void addInstanceDatas( const ObjectDataIStreamDeque&, 
                                       const uint128_t& startVersion );
    private:
        /** The current version. */
        uint128_t _version;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentIStream;

        /** The change queue. */
        lunchbox::MTQueue< ObjectDataIStream* > _queuedVersions;

        /** Cached input streams (+decompressor) */
        lunchbox::Pool< ObjectDataIStream, true > _iStreamCache;

        /** The instance identifier of the master object. */
        uint32_t _masterInstanceID;

        /** The output stream for slave object commits. */
        ObjectSlaveDataOStream _ostream;

        /** The node holding the master object. */
        NodePtr _master;

        void _syncToHead();
        void _releaseStream( ObjectDataIStream* stream );

        /** Apply the data in the input stream to the object */
        virtual void _unpackOneVersion( ObjectDataIStream* is );

        /* The command handlers. */
        bool _cmdData( Command& command );

        LB_TS_VAR( _cmdThread );
        LB_TS_VAR( _rcvThread );
    };
}

#endif // CO_VERSIONEDSLAVECM_H
