
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
            TYPE_CHANNEL = eqNet::Object::TYPE_INTERNAL, // 256
            TYPE_WINDOW,
            TYPE_PIPE,
            TYPE_NODE,
            TYPE_MATRIX4F,
            TYPE_MATRIX4D,
            TYPE_FRAME,
            TYPE_FRAMEDATA
        };
    };
}

#endif // EQ_OBJECT_H
