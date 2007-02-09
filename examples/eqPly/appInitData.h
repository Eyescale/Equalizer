
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch>
   All rights reserved. */

#ifndef EQ_PLY_APPINITDATA_H
#define EQ_PLY_APPINITDATA_H

#include "initData.h"

class FrameData;

class AppInitData : public InitData
{
public:
    const std::string& getTrackerPort() const { return _trackerPort; }

    void parseArguments( int argc, char** argv, FrameData& frameData );

private:
    std::string _trackerPort;
};

#endif // EQ_PLY_APPINITDATA_H
