
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

