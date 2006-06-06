
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_H
#define EQ_PLY_H

#include "colorVertex.h"
#include "normalFace.h"
#include "plyModel.h"

#include <eq/eq.h>

enum ObjectType
{
    TYPE_INITDATA  = eq::Object::TYPE_MANAGED_CUSTOM,
    TYPE_FRAMEDATA = eq::Object::TYPE_VERSIONED_CUSTOM
};

typedef PlyModel< NormalFace<ColorVertex> > Model;


#endif // EQ_PLY_H

