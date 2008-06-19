
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com>
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
        bool               getOrtho()       const { return _ortho;  }
        uint32_t           getMaxFrames()   const { return _maxFrames; }

        const LocalInitData& operator = ( const LocalInitData& from );

    private:
        uint32_t    _maxFrames;
        bool        _isResident;
        bool        _ortho;
    };
}

#endif // EVOLVE_LOCALINITDATA_H
