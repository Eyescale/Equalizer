
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent ) : eq::Channel( parent ), _font( parent )
            {}

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

    private:
        void _drawModel( const Model* model );
        void _drawLogo();
        void _initFrustum( vmml::FrustumCullerf& frustum, 
                           const vmml::Vector4f& boundingSphere );

        const FrameData::Data& _getFrameData() const;

        eq::util::BitmapFont _font;
    };
}



#endif // EQ_PLY_CHANNEL_H

