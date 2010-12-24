
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "version.h"             // enum

#include <co/base/mtQueue.h>     // member
#include <co/base/pool.h>        // member
#include <co/base/thread.h>      // thread-safety macro

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
        virtual uint32_t commitNB();
        virtual uint128_t commitSync( const uint32_t commitID );

        virtual void setAutoObsolete( const uint32_t ) { EQDONTCALL; }
        virtual uint32_t getAutoObsolete() const { EQDONTCALL; return 0; }

        virtual uint128_t sync( const uint128_t& version );

        virtual uint128_t getHeadVersion() const;
        virtual uint128_t getVersion() const { return _version; }
        virtual uint128_t getOldestVersion() const { return _version; }
        //@}

        virtual bool isMaster() const { return false; }

        virtual uint32_t getMasterInstanceID() const {return _masterInstanceID;}
        virtual void setMasterNode( NodePtr node ) { _master = node; }
        virtual const NodeID& getMasterNodeID() const;

        virtual uint128_t addSlave( Command& )
            { EQDONTCALL; return VERSION_INVALID; }
        virtual void removeSlave( NodePtr ) { EQDONTCALL; }

        virtual void applyMapData( const uint128_t& version );
        virtual void addInstanceDatas( const ObjectDataIStreamDeque&, 
                                       const uint128_t& startVersion );
        virtual const Object* getObject( ) const { return _object; }

    private:
        /** The managed object. */
        Object* const _object;

        /** The current version. */
        uint128_t _version;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentIStream;

        /** The change queue. */
        co::base::MTQueue< ObjectDataIStream* > _queuedVersions;

        /** Cached input streams (+decompressor) */
        co::base::Pool< ObjectDataIStream, true > _iStreamCache;

        /** The instance identifier of the master object. */
        uint32_t _masterInstanceID;

        /** The output stream for slave object commits. */
        ObjectSlaveDataOStream _ostream;

        /** The node holding the master object. */
        NodePtr _master;

        void _syncToHead();

        /** Apply the data in the input stream to the object */
        virtual void _unpackOneVersion( ObjectDataIStream* is );

        /* The command handlers. */
        bool _cmdInstance( Command& command );
        bool _cmdDelta( Command& command );
        bool _cmdCommit( Command& command );

        EQ_TS_VAR( _cmdThread );
    };
}

#endif // CO_VERSIONEDSLAVECM_H
