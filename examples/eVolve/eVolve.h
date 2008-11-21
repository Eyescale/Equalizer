
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_H
#define EVOLVE_H

#include <eq/eq.h>

#include "rawVolModelRenderer.h"


/** The Equalizer Volume Rendering Example */
namespace eVolve
{
    class LocalInitData;
    typedef RawVolumeModelRenderer Renderer;


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

    enum LogTopics
    {
        LOG_STATS = eq::LOG_CUSTOM      // 65536
    };
}

#endif // EVOLVE_H

