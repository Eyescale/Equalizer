
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include <eq/eq.h>

class FrameData;
class InitData;

namespace eqPly
{
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent ) : eq::Channel( parent ) {}

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );

    private:
        void _drawModel( const Model* model );
        void _drawLogo();
        void _initFrustum( vmml::FrustumCullerf& frustum, 
                           const vmml::Vector4f& boundingSphere );
    };
}



#endif // EQ_PLY_CHANNEL_H

