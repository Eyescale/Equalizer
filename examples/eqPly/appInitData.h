
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch>
   All rights reserved. */

#ifndef EQ_PLY_APPINITDATA_H
#define EQ_PLY_APPINITDATA_H

class AppInitData : public InitData
{
public:
    void setTrackerPort( const std::string& trackerPort )
    {
        _trackerPort = trackerPort;
    }
    const std::string& getTrackerPort() const { return _trackerPort; }

private:
    std::string _trackerPort;
};

#endif // EQ_PLY_APPINITDATA_H
