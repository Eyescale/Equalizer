
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_PLY_LOCALINITDATA_H
#define EQ_PLY_LOCALINITDATA_H

#include "initData.h"

class FrameData;

namespace eqPly
{
    /**
     * Manages the argument parsing and non-distributed part of the
     * initialization data.
     */
    class LocalInitData : public InitData
    {
    public:
        LocalInitData();

        void parseArguments( const int argc, char** argv );

        const std::string& getTrackerPort() const { return _trackerPort; }
        const std::string& getLogFilename() const { return _logFilename; }
        bool               useColor()       const { return _color; }
        bool               isResident()     const { return _isResident; }
        uint32_t           getMaxFrames()   const { return _maxFrames; }
        
        const std::vector< std::string >& getFilenames() const
            { return _filenames; }

        const LocalInitData& operator = ( const LocalInitData& from );

    private:
        std::string _trackerPort;
        std::vector< std::string > _filenames;
        std::string _logFilename;
        uint32_t    _maxFrames;
        bool        _color;
        bool        _isResident;
    };
}

#endif // EQ_PLY_LOCALINITDATA_H
