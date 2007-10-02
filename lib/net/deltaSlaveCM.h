
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DELTASLAVECM_H
#define EQNET_DELTASLAVECM_H

#include "fullSlaveCM.h"     // base class

#include <eq/net/commandQueue.h> // member
#include <eq/net/object.h>       // nested enum (Object::Version)
#include <eq/base/idPool.h>      // for EQ_ID_INVALID

namespace eqNet
{
    class ObjectDeltaDataIStream;
    class Node;

    /** 
     * An object change manager handling full versions and deltas for slave
     * instances.
     */
    class DeltaSlaveCM : public FullSlaveCM
    {
    public:
        DeltaSlaveCM( Object* object );
        virtual ~DeltaSlaveCM();

    protected:
        virtual void _unpackOneVersion( ObjectDataIStream* is );
    };
}

#endif // EQNET_DELTASLAVECM_H
