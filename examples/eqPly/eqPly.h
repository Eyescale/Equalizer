
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_H
#define EQ_PLY_H

#include "colorVertex.h"
#include "normalFace.h"
#include "plyModel.h"

#include <eq/eq.h>

enum ObjectType
{
    TYPE_INITDATA  = eqNet::Object::TYPE_CUSTOM,
    TYPE_FRAMEDATA
};

typedef PlyModel< NormalFace<ColorVertex> > Model;


#endif // EQ_PLY_H

