
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

#ifndef EQNET_VERSIONEDSLAVECM_H
#define EQNET_VERSIONEDSLAVECM_H

#include <eq/net/objectCM.h>     // base class
#include <eq/net/commandQueue.h> // member
#include <eq/net/version.h>      // enum
#include <eq/base/idPool.h>      // for EQ_ID_INVALID

namespace eq
{
namespace net
{
    class Node;
    class ObjectDataIStream;
    class ObjectDeltaDataIStream;

    /** 
     * An object change manager handling changes for versioned slave instances.
     */
    class VersionedSlaveCM : public ObjectCM
    {
    public:
        VersionedSlaveCM( Object* object, uint32_t masterInstanceID );
        virtual ~VersionedSlaveCM();

        virtual void makeThreadSafe();

        /**
         * @name Versioning
         */
        //@{
        virtual uint32_t commitNB() { EQDONTCALL; return EQ_ID_INVALID; }
        virtual uint32_t commitSync( const uint32_t commitID )
            { EQDONTCALL; return VERSION_NONE; }

        virtual void obsolete( const uint32_t version ) { EQDONTCALL; }

        virtual void setAutoObsolete( const uint32_t count,
                                      const uint32_t flags ) { EQDONTCALL; }
        virtual uint32_t getAutoObsoleteCount() const
            { EQDONTCALL; return 0; }

        virtual uint32_t sync( const uint32_t version );

        virtual uint32_t getHeadVersion() const;
        virtual uint32_t getVersion() const { return _version; }
        virtual uint32_t getOldestVersion() const { return _version; }
        //@}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const {return _masterInstanceID;}

        virtual uint32_t addSlave( Command& command )
            { EQDONTCALL; return VERSION_INVALID; }
        virtual void removeSlave( NodePtr node ) { EQDONTCALL; }
        virtual void addOldMaster( NodePtr node, const uint32_t instanceID )
            { EQDONTCALL }

        virtual void applyMapData();
        virtual void addInstanceDatas( const InstanceDataDeque* cache, 
                                       const uint32_t startVersion );

    private:
        /** The managed object. */
        Object* _object;

        /** The current version. */
        uint32_t _version;

        /** The mutex, if thread safety is enabled. */
        base::Lock* _mutex;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentIStream;

        /** The change queue. */
        base::MTQueue< ObjectDataIStream* > _queuedVersions;

        /** The instance identifier of the master object. */
        uint32_t _masterInstanceID;

        void _syncToHead();

        /** Apply the data in the input stream to the object */
        virtual void _unpackOneVersion( ObjectDataIStream* is );

        /* The command handlers. */
        CommandResult _cmdInstance( Command& command );
        CommandResult _cmdDelta( Command& command );
        CommandResult _cmdVersion( Command& command );

        CHECK_THREAD_DECLARE( _thread );
        CHECK_THREAD_DECLARE( _cmdThread );
    };
}
}

#endif // EQNET_VERSIONEDSLAVECM_H
