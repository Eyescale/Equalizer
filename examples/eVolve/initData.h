
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_INITDATA_H
#define EVOLVE_INITDATA_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eVolve
{
    class InitData : public eq::net::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )   { _frameDataID = id; }

        uint32_t           getFrameDataID()  const { return _frameDataID;  }
        eq::WindowSystem   getWindowSystem() const { return _windowSystem; }
        uint32_t           getPrecision()    const { return _precision;    }
        float              getBrightness()   const { return _brightness;   }
        float              getAlpha()        const { return _alpha;        }
        const std::string& getFilename()     const { return _filename;     }

    protected:
        virtual void getInstanceData(   eq::net::DataOStream& os );
        virtual void applyInstanceData( eq::net::DataIStream& is );

        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _windowSystem = windowSystem; }
        void setPrecision( const uint32_t precision ){ _precision = precision; }
        void setBrightness( const float brightness ) {_brightness = brightness;}
        void setAlpha( const float alpha )           { _alpha = alpha;}
        void setFilename( const std::string& filename ) { _filename = filename;}

    private:
        uint32_t         _frameDataID;
        eq::WindowSystem _windowSystem;
        uint32_t         _precision;
        float            _brightness;
        float            _alpha;
        std::string      _filename;
    };
}


#endif // EVOLVE_INITDATA_H

