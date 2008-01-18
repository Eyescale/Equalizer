
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_H
#define EVOLVE_H

#include "rawVolModelRenderer.h"

#include <eq/eq.h>

namespace eVolve
{
    typedef RawVolumeModelRenderer Model;

    class LocalInitData;

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
}

#endif // EVOLVE_H

