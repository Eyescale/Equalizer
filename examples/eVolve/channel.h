
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_CHANNEL_H
#define EVOLVE_CHANNEL_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>


namespace eVolve
{
    class InitData;

    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent );

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );

        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

        void clearViewport( const eq::PixelViewport &pvp );

        void frameClear( const uint32_t frameID );

    private:

        void _startAssemble();
        void _finishAssemble();

        void _orderFrames( eq::FrameVector& frames );

        void _drawLogo();

        void _calcMVandITMV( vmml::Matrix4d& modelviewM, 
                             vmml::Matrix3d& modelviewITM ) const;

        const FrameData::Data& _getFrameData() const;

        vmml::Vector4f _bgColor; // background color

        enum BGColorMode
        {
            BG_SOLID_BLACK    = 0,
            BG_SOLID_COLORED  = 1
        } 
            _bgColorMode;

        eq::Frame _frame;     //!< Readback buffer for DB compositing
        eq::Range _drawRange; //!< The range from the last draw of this frame
    };

}

#endif // EVOLVE_CHANNEL_H

