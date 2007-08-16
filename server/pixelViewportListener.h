
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PIXELVIEWPORT_LISTENER_H
#define EQS_PIXELVIEWPORT_LISTENER_H

#include <eq/base/pixelViewport.h>

namespace eqs
{
    /**
     * A generic listener on pixel viewport changes.
     */
    class PixelViewportListener
    {
    public:
        virtual void notifyPVPChanged( const eq::PixelViewport& pvp ){}
    };
};
#endif // EQS_PIXELVIEWPORT_LISTENER_H
