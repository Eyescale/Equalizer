
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NULLCM_H
#define EQNET_NULLCM_H

#include <eq/net/objectCM.h> // base class
#include <eq/net/object.h>   // nested enum (Object::Version)
#include <eq/base/idPool.h>  // for EQ_ID_INVALID

namespace eqNet
{
    class Node;

    /** 
     * The object change manager doing nothing, for unmapped objects.
     */
    class NullCM : public ObjectCM
    {
    public:
        NullCM() {}
        virtual ~NullCM() {}

        virtual void makeThreadSafe() {}

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
        virtual bool syncInitial()                 { EQDONTCALL; return false; }


        virtual uint32_t getHeadVersion() const { return Object::VERSION_NONE; }
        virtual uint32_t getVersion() const     { return Object::VERSION_NONE; }
        //*}

        virtual bool isMaster() const { return false; }

        virtual void addSlave( eqBase::RefPtr<Node> slave, 
                               const uint32_t instanceID )    { EQDONTCALL; }
        virtual void removeSlave( eqBase::RefPtr<Node> node ) { EQDONTCALL; }

    private:
    };
}

#endif // EQNET_NULLCM_H
