
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FLIP_H
#define EQ_FLIP_H

#include "colorVertex.h"
#include "normalFace.h"
#include "plyModel.h"

#include <eq/eq.h>

enum ObjectType
{
    OBJECT_INITDATA  = eqNet::MOBJECT_CUSTOM,
    OBJECT_FRAMEDATA 
};

typedef PlyModel< NormalFace<ColorVertex> > Model;


#endif // EQ_FLIP_H

