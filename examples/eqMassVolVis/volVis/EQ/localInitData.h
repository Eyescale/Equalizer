
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOL__LOCALINITDATA_H
#define MASS_VOL__LOCALINITDATA_H

#include "initData.h"
#include "../renderer/rendererTypes.h"

//class FrameData;

namespace massVolVis
{


class LocalInitData : public InitData
{
public:
    LocalInitData();

    void parseArguments( int argc, char** argv );

    bool            isResident()        const { return _isResident;     }
    bool            getOrtho()          const { return _ortho;          }
    uint32_t        getMaxFrames()      const { return _maxFrames;      }
    RendererType    getRendererType()   const { return _rType;          }
    bool            dumpStatistics()    const { return _dumpStatistics; }

    const std::string& getPathFilename()const { return _pathFilename; }

    const LocalInitData& operator = ( const LocalInitData& from );

private:
    uint32_t        _maxFrames;
    bool            _isResident;
    bool            _ortho;
    RendererType    _rType;
    std::string     _pathFilename;
    bool            _dumpStatistics;
};


}//namespace massVolVis

#endif // MASS_VOL__LOCALINITDATA_H
