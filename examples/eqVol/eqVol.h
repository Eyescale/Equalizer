
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

        /** @sa eqNet::Node::initLocal() */
        virtual bool initLocal( int argc, char** argv );
        
        /** Run an eqVol instance. */
        int run();
        
    protected:
        int runMainloop();
        int runClient();
        
    private:
        const LocalInitData& _initData;
    };
}

#endif // EQ_VOL_H

