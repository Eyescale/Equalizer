
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_PLY_LOCALINITDATA_H
#define EQ_PLY_LOCALINITDATA_H

#include "initData.h"

class FrameData;

class LocalInitData : public InitData
{
public:
    LocalInitData();

    const std::string& getTrackerPort() const { return _trackerPort; }
    bool               useColor()       const { return _color; }
    bool               isApplication()  const { return _isApplication; }
    uint16_t           getClientPort()  const { return _clientPort; }

    void parseArguments( int argc, char** argv );

private:
    std::string _trackerPort;
    uint16_t    _clientPort;
    bool        _color;
    bool        _isApplication;
};

#endif // EQ_PLY_APPINITDATA_H
