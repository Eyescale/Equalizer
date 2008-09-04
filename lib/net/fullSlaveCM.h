
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_FULLSLAVECM_H
#define EQNET_FULLSLAVECM_H

#include "staticSlaveCM.h"     // base class

#include <eq/net/commandQueue.h> // member
#include <eq/net/object.h>       // nested enum (Object::Version)
#include <eq/base/idPool.h>      // for EQ_ID_INVALID

namespace eq
{
namespace net
{
    class Node;
    class ObjectDataIStream;
    class ObjectDeltaDataIStream;

    /** 
     * An object change manager handling full instance versions for slave
     * instances.
     */
    class FullSlaveCM : public StaticSlaveCM
    {
    public:
        FullSlaveCM( Object* object, uint32_t masterInstanceID );
        virtual ~FullSlaveCM();

        virtual void notifyAttached();
        virtual void makeThreadSafe();

        /**
         * @name Versioning
         */
        //*{
        virtual uint32_t commitNB() { EQDONTCALL; return EQ_ID_INVALID; }
        virtual uint32_t commitSync( const uint32_t commitID )
            { EQDONTCALL; return Object::VERSION_NONE; }

        virtual void obsolete( const uint32_t version ) { EQDONTCALL; }

        virtual void setAutoObsolete( const uint32_t count,
                                      const uint32_t flags ) { EQDONTCALL; }
        virtual uint32_t getAutoObsoleteCount() const
            { EQDONTCALL; return 0; }

        virtual bool sync( const uint32_t version );

        virtual uint32_t getHeadVersion() const;
        virtual uint32_t getVersion() const { return _version; }
        virtual uint32_t getOldestVersion() const { return _version; }
        //*}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const {return _masterInstanceID;}

        virtual void addSlave( NodePtr slave, const uint32_t instanceID,
                               const uint32_t version ) { EQDONTCALL; }
        virtual void removeSlave( NodePtr node ) { EQDONTCALL; }

        virtual void applyMapData();

    protected:
        /** The current version. */
        uint32_t _version;

        /** The change queue. */
        base::MTQueue< ObjectDataIStream* > _queuedVersions;

        /** Apply the data in the input stream to the object */
        virtual void _unpackOneVersion( ObjectDataIStream* is );

    private:
        /** The mutex, if thread safety is enabled. */
        base::Lock* _mutex;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentDeltaStream;

        /** The instance identifier of the master object. */
        uint32_t _masterInstanceID;

        void _syncToHead();

        /* The command handlers. */
        CommandResult _cmdDeltaData( Command& command );
        CommandResult _cmdDelta( Command& command );

        CHECK_THREAD_DECLARE( _thread );
    };
}
}

#endif // EQNET_FULLSLAVECM_H
