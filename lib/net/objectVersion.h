
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTVERSION_H
#define EQNET_OBJECTVERSION_H

#include <eq/base/base.h>

#include <iostream>

namespace eq
{
namespace net
{
    class Object;

    /** A helper struct bundling an object identifier and version. */
    struct ObjectVersion
    {
        ObjectVersion();
        ObjectVersion( const uint32_t id, const uint32_t version );
        ObjectVersion( const Object* object );
        ObjectVersion& operator = ( const Object* object );
        
        uint32_t id;
        uint32_t version;
    };

    inline std::ostream& operator << (std::ostream& os, const ObjectVersion& ov)
    {
        os << " id " << ov.id << " v" << ov.version;
        return os;
    }
}
}

#endif // EQNET_OBJECT_H
