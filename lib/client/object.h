
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OBJECT_H
#define EQ_OBJECT_H

#include <eq/net/object.h>

namespace eq
{
    /** just for the type enum right now. */
    class Object
    {
    public:
        enum Type
        {
            TYPE_CHANNEL          = eqNet::Object::TYPE_MANAGED_CUSTOM,
            TYPE_WINDOW,
            TYPE_PIPE,
            TYPE_NODE,
            TYPE_MANAGED_CUSTOM,  // 6
            TYPE_MATRIX4F         = eqNet::Object::TYPE_VERSIONED_CUSTOM,
            TYPE_FRAME,
            TYPE_FRAMEBUFFER,
            TYPE_VERSIONED_CUSTOM
        };
    };
}

#endif // EQ_OBJECT_H
