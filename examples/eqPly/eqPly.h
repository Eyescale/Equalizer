
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_H
#define EQ_PLY_H

#include <eq/eq.h>

#include "vertexBufferDist.h"
#include "vertexBufferRoot.h"


/** The Equalizer polygonal rendering example. */
namespace eqPly
{
    class LocalInitData;

    typedef mesh::VertexBufferRoot    Model;
    typedef VertexBufferDist          ModelDist;

    /** The EqPly application instance */
    class EqPly : public eq::Client
    {
    public:
        EqPly( const LocalInitData& initData );
        virtual ~EqPly() {}

        /** Run an eqPly instance. */
        int run();
        
    protected:
        /** @sa eq::Client::clientLoop. */
        virtual bool clientLoop();
        
    private:
        const LocalInitData& _initData;
    };

    enum LogTopics
    {
        LOG_STATS = eq::LOG_CUSTOM << 0, // 65536
        LOG_CULL  = eq::LOG_CUSTOM << 1  // 131072
    };
}

#endif // EQ_PLY_H

