
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectVersion.h"

#include "object.h"
#include <eq/base/idPool.h>

namespace eq
{
namespace net
{
ObjectVersion::ObjectVersion()
        : id( EQ_ID_INVALID ), version( Object::VERSION_NONE )
{}

ObjectVersion::ObjectVersion( const uint32_t id_, const uint32_t version_ )
        : id( id_ ), version( version_ )
{}

ObjectVersion::ObjectVersion( const Object* object )
        : id( object->getID( )), 
          version( object->getVersion( )) 
{
}

ObjectVersion& ObjectVersion::operator = ( const Object* object )
{
    if( object )
    {
        id = object->getID();
        version = object->getVersion();
    }
    else
    {
        id = EQ_ID_INVALID;
        version = Object::VERSION_NONE;
    }

    return *this;
}

}
}
