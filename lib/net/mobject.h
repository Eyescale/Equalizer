
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_MOBJECT_H
#define EQNET_MOBJECT_H

#include "object.h"

#include <eq/base/referenced.h>
#include <eq/base/refPtr.h>

namespace eqNet
{
    class  Node;
    struct MobjectPacket;

    enum MobjectType
    {
        MOBJECT_UNDEFINED,
        MOBJECT_EQNET_BARRIER,
        MOBJECT_CUSTOM
    };

    /** 
     * A managed object, which has a master node and can be automatically
     * instatiated and destroyed on other nodes.
     */
    class Mobject : public Object, public eqBase::Referenced
    {
    public:
        enum InstState
        {
            INST_UNKNOWN = 0,
            INST_GETMASTERID,
            INST_GETMASTER,
            INST_GOTMASTER,
            INST_INIT,
            INST_ERROR
        };

        Mobject( const uint32_t typeID ) : _typeID( typeID ) {}
        virtual ~Mobject(){}
        
        /** 
         * Return the instance information about this mobject.
         *
         * The returned pointer has to point to at least size bytes of memory
         * allocated by the mobject. It can be free'd or reused on the next call
         * to this mobject.
         * 
         * @param typeID the type identifier of this mobject.
         * @param size the size of the returned data.
         * @return the instance data.
         */
        virtual const void* getInstanceData( uint64_t* size ) = 0;

    protected:
        /** 
         * @return if this instance is the master version.
         */
        bool isMaster() const { return _master; }

        /** 
         * Add a subscribed slave to the mobject.
         * 
         * @param slave the slave.
         */
        void addSlave( eqBase::RefPtr<Node> slave ) 
            { _slaves.push_back( slave ); }

        /** 
         * @return the vector of registered slaves.
         */
        std::vector< eqBase::RefPtr<Node> >& getSlaves()
            { return _slaves; }

    private:
        /** Indicates if this instance is the copy on the server node. */
        friend class Session;
        bool         _master;
        
        /** The list of subsribed slaves, kept on the master only. */
        std::vector< eqBase::RefPtr<Node> > _slaves;

        /** The type identifier of the object. */
        uint32_t _typeID;
    };
}

#endif // EQNET_MOBJECT_H
