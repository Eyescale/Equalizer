
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "versionedObject.h"

#include "packets.h"

#include <eq/base/log.h>
#include <iostream>

using namespace eqNet;
using namespace std;

VersionedObject::VersionedObject()
        : _version( EQ_UNDEFINED_UINT32 )
{
}

VersionedObject::~VersionedObject()
{
}

uint32_t VersionedObject::commit()
{
    if( !isMaster( ))
        return EQ_UNDEFINED_UINT32;

    
    //VersionedObjectSyncPacket packet( getSession()->getID(), getID( ));
    string delta;
    pack( delta );
    
    
    EQUNIMPLEMENTED;
}

bool VersionedObject::sync( const uint32_t version, const float timeout )
{
    EQUNIMPLEMENTED;
}

void VersionedObject::sync()
{
    EQUNIMPLEMENTED;
}

uint32_t VersionedObject::getHeadVersion() const
{
    EQUNIMPLEMENTED;
}
