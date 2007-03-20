
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_H
#define EQ_PLY_H

#include "colorVertex.h"
#include "normalFace.h"
#include "plyModel.h"

#include <eq/eq.h>

typedef PlyModel< NormalFace<ColorVertex> > Model;

class LocalInitData;

class EqPly : public eq::Client
{
public:
    EqPly( const LocalInitData& initData );
    virtual ~EqPly() {}

    /** @sa eqNet::Node::initLocal() */
    virtual bool initLocal( int argc, char** argv );

    /** Run an eqPly instance. */
    int run();

protected:
    int runApplication();
    int runClient();

private:
    const LocalInitData& _initData;
};

#endif // EQ_PLY_H

