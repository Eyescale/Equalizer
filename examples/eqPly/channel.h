
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"
#include "frameData.h"

#include <eq/eq.h>


namespace eqPly
{
    class FrameData;
    class InitData;

    /**
     * The rendering entity, updating a part of a Window.
     */
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent ) : eq::Channel( parent ) {}

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

    private:
        void _drawModel( const Model* model );
        void _drawLogo();
        void _initFrustum( vmml::FrustumCullerf& frustum, 
                           const vmml::Vector4f& boundingSphere );

        const FrameData& _getFrameData() const;
    };
}



#endif // EQ_PLY_CHANNEL_H

