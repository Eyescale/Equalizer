
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_CHANNEL_H
#define EQ_VOL_CHANNEL_H

#include "eVolve.h"

#include <eq/eq.h>


namespace eVolve
{
    class FrameData;
    class InitData;
    struct Frame;

    class Channel : public eq::Channel
    {
    public:
        Channel();
        bool _perspective;  // perspective/ortogonal projection
        bool _useCg;        // Cg / GLSL shaders 

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameDraw( const uint32_t frameID );

        virtual void frameAssemble( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );

        void applyFrustum() const;

        void clearViewport( const eq::PixelViewport &pvp );

        void frameClear( const uint32_t frameID );

    private:

        void _startAssemble();
        void _finishAssemble();

        void _orderFrames( std::vector< Frame >& frames );

        void _drawLogo();

        void _calcMVandITMV( vmml::Matrix4d& modelviewM, 
                             vmml::Matrix3d& modelviewITM ) const;

        const FrameData& _getFrameData() const;

        //
        struct curFrData
        {
            uint32_t    frameID;
            eq::Range   lastRange;
        }
            _curFrData;

        vmml::Vector4f _bgColor;
        
        eq::Image _image; //!< buffer for readback in case of DB compositing

        GLuint _slicesListID;     //!< display list for hexagonals
    };

}

#endif // EQ_VOL_CHANNEL_H

