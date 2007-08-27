
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_VOL_LOCALINITDATA_H
#define EQ_VOL_LOCALINITDATA_H

#include "initData.h"

class FrameData;

namespace eqVol
{
    class LocalInitData : public InitData
    {
    public:
        LocalInitData();

        void parseArguments( int argc, char** argv );

        const std::string& getTrackerPort() const { return _trackerPort; }
        bool               useColor()       const { return _color; }
        bool               isApplication()  const { return _isApplication; }
        uint16_t           getClientPort()  const { return _clientPort; }
        uint32_t           getMaxFrames()   const { return _maxFrames; }

    private:
        std::string _trackerPort;
        uint32_t    _maxFrames;
        uint16_t    _clientPort;
        bool        _color;
        bool        _isApplication;
    };
}

#endif // EQ_VOL_LOCALINITDATA_H
