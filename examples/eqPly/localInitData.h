
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
        const std::string& getPathFilename()const { return _pathFilename; }
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
        std::string _pathFilename;
        uint32_t    _maxFrames;
        bool        _color;
        bool        _isResident;
    };
}

#endif // EQ_PLY_LOCALINITDATA_H
