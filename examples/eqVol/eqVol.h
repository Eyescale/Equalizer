
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_H
#define EQ_VOL_H

#include "rawVolModel.h"

#include <eq/eq.h>

namespace eqVol
{
    typedef RawVolumeModel Model;

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

#endif // EQ_VOL_H

