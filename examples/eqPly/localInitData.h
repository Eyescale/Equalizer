
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_PLY_LOCALINITDATA_H
#define EQ_PLY_LOCALINITDATA_H

#include "initData.h"

class FrameData;

namespace eqPly
{
    class LocalInitData : public InitData
    {
    public:
        LocalInitData();

        void parseArguments( const int argc, char** argv );

        const std::string& getTrackerPort() const { return _trackerPort; }
        bool               useColor()       const { return _color; }
        bool               isResident()     const { return _isResident; }
        uint32_t           getMaxFrames()   const { return _maxFrames; }
        const std::string& getFilename()    const { return _filename; }

        const LocalInitData& operator = ( const LocalInitData& from );

    private:
        std::string _trackerPort;
        uint32_t    _maxFrames;
        bool        _color;
        bool        _isResident;
        std::string _filename;
    };
}

#endif // EQ_PLY_LOCALINITDATA_H
