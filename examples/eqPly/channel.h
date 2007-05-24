
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include "plyModel.h"

#include <eq/eq.h>

class FrameData;
class InitData;

namespace eqPly
{
    class Channel : public eq::Channel
    {
    public:

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameDraw( const uint32_t frameID );

    private:
        static void _drawBBoxCB( Model::BBox *bbox, void *userData );

        void _drawBBox( const Model::BBox* bbox );
        void _drawLogo();
        void _initFrustum( vmml::FrustumCullerf& frustum );
    };
}



#endif // EQ_PLY_CHANNEL_H

