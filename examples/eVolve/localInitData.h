
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EVOLVE_LOCALINITDATA_H
#define EVOLVE_LOCALINITDATA_H

#include "initData.h"

class FrameData;

namespace eVolve
{
    class LocalInitData : public InitData
    {
    public:
        LocalInitData();

        void parseArguments( int argc, char** argv );

        bool               isResident()     const { return _isResident; }
        uint32_t           getMaxFrames()   const { return _maxFrames; }

        const LocalInitData& operator = ( const LocalInitData& from );

    private:
        uint32_t    _maxFrames;
        bool        _isResident;
    };
}

#endif // EVOLVE_LOCALINITDATA_H
