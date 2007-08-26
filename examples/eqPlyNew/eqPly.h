
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_H
#define EQ_PLY_H

//#include "colorVertex.h"
//#include "normalFace.h"
//#include "plyModel.h"

#include <eq/eq.h>

#include "vertexBufferRoot.h"

//typedef PlyModel< NormalFace<ColorVertex> > Model;
typedef mesh::VertexBufferRoot    Model;

namespace eqPly
{
    class LocalInitData;

    class Application : public eq::Client
    {
    public:
        Application( const LocalInitData& initData );
        virtual ~Application() {}

        /** Run an eqPly instance. */
        int run();
        
    protected:
        /** @sa eq::Client::clientLoop. */
        virtual bool clientLoop();
        
    private:
        const LocalInitData& _initData;
    };
}

#endif // EQ_PLY_H

