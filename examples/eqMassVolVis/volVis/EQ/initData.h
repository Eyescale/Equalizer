
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */


#ifndef MASS_VOL__INITDATA_H
#define MASS_VOL__INITDATA_H

//#include "eVolve.h"
//#include "frameData.h"

#include <eq/eq.h>

namespace massVolVis
{


class InitData : public co::Object
{
public:
    InitData();
    virtual ~InitData();

    void setFrameDataId(  const lunchbox::UUID& id ) { _frameDataId  = id; }
    void setVolumeInfoId( const lunchbox::UUID& id ) { _volumeInfoId = id; }

    lunchbox::UUID     getFrameDataId()  const { return _frameDataId;  }
    lunchbox::UUID     getVolumeInfoId() const { return _volumeInfoId;  }
    eq::WindowSystem   getWindowSystem() const { return _windowSystem; }
    const std::string& getFilename()     const { return _filename;     }

protected:
    virtual void getInstanceData(   co::DataOStream& os );
    virtual void applyInstanceData( co::DataIStream& is );

    void setWindowSystem( const eq::WindowSystem windowSystem ) { _windowSystem = windowSystem; }
    void setFilename(     const std::string&     filename     ) { _filename     = filename;     }

private:
    lunchbox::UUID   _frameDataId;
    lunchbox::UUID   _volumeInfoId;
    eq::WindowSystem _windowSystem;
    std::string      _filename;
};


}//namespace massVolVis


#endif // MASS_VOL__INITDATA_H

