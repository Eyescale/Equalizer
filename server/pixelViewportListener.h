
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_PIXELVIEWPORT_LISTENER_H
#define EQSERVER_PIXELVIEWPORT_LISTENER_H

#include <eq/client/pixelViewport.h>

namespace eq
{
namespace server
{
    /**
     * A generic listener on pixel viewport changes.
     */
    class PixelViewportListener
    {
    public:
        virtual ~PixelViewportListener(){}

        virtual void notifyPVPChanged( const eq::PixelViewport& pvp ){}
    };
}
}
#endif // EQSERVER_PIXELVIEWPORT_LISTENER_H
