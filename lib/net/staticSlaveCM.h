
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_STATICSLAVECM_H
#define EQNET_STATICSLAVECM_H

#include <eq/net/objectCM.h>     // base class
#include <eq/net/command.h>      // member
#include <eq/net/object.h>       // nested enum (Object::Version)
#include <eq/base/idPool.h>      // for EQ_ID_INVALID
#include <eq/base/monitor.h>     // member

namespace eqNet
{
    class Node;
    class ObjectDataIStream;

    /** 
     * An object change manager handling static object slave instances.
     */
    class StaticSlaveCM : public ObjectCM
    {
    public:
        StaticSlaveCM( Object* object );
        virtual ~StaticSlaveCM();

        virtual void notifyAttached();
        virtual void makeThreadSafe(){}

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

        virtual bool sync( const uint32_t version ) { EQDONTCALL; return false;}

        virtual uint32_t getHeadVersion() const { return Object::VERSION_NONE; }
        virtual uint32_t getVersion() const     { return Object::VERSION_NONE; }
        //*}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const { return EQ_ID_INVALID; }

        virtual void addSlave( eqBase::RefPtr<Node> slave, 
                               const uint32_t instanceID )    { EQDONTCALL; }
        virtual void removeSlave( eqBase::RefPtr<Node> node ) { EQDONTCALL; }

        virtual void applyMapData();

    protected:
        /** The managed object. */
        Object* _object;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentIStream;

    private:
        /* The command handlers. */
        CommandResult _cmdInstanceData( Command& command );
        CommandResult _cmdInstance( Command& command );
    };
}

#endif // EQNET_STATICSLAVECM_H
