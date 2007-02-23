
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_STATICMASTERCM_H
#define EQNET_STATICMASTERCM_H

#include <eq/net/objectCM.h> // base class
#include <eq/net/object.h>   // nested enum (Object::Version)
#include <eq/base/idPool.h>  // for EQ_ID_INVALID

#include <deque>

namespace eqNet
{
    class Node;

    /** 
     * An object change manager handling a static master instance.
     */
    class StaticMasterCM : public ObjectCM
    {
    public:
        StaticMasterCM( Object* object );
        virtual ~StaticMasterCM();

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

        virtual bool sync( const uint32_t version ){ EQDONTCALL; return false; }

        virtual const void* getInitialData( uint64_t* size, uint32_t* version );
        virtual void applyInitialData( const void* data, const uint64_t size,
                                       const uint32_t version ) { EQDONTCALL; }

        virtual uint32_t getHeadVersion() const { return Object::VERSION_NONE; }
        virtual uint32_t getVersion() const     { return Object::VERSION_NONE; }
        //*}

        virtual bool isMaster() const { return true; }
        virtual void addSlave( eqBase::RefPtr<Node> node, 
                               const uint32_t instanceID ){}
        virtual void removeSlave( eqBase::RefPtr<Node> node ) {}

    private:
        /** The managed object. */
        Object* _object;
    };
}

#endif // EQNET_STATICMASTERCM_H
