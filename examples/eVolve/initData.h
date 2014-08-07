
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EVOLVE_INITDATA_H
#define EVOLVE_INITDATA_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eVolve
{
    class InitData : public co::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const eq::uint128_t& id )   { _frameDataID = id; }

        eq::uint128_t      getFrameDataID()  const { return _frameDataID;  }
        const std::string& getWindowSystem() const { return _windowSystem; }
        uint32_t           getPrecision()    const { return _precision;    }
        float              getBrightness()   const { return _brightness;   }
        float              getAlpha()        const { return _alpha;        }
        const std::string& getFilename()     const { return _filename;     }

    protected:
        virtual void getInstanceData(   co::DataOStream& os );
        virtual void applyInstanceData( co::DataIStream& is );

        void setWindowSystem( const std::string& windowSystem )
            { _windowSystem = windowSystem; }
        void setPrecision( const uint32_t precision ){ _precision = precision; }
        void setBrightness( const float brightness ) {_brightness = brightness;}
        void setAlpha( const float alpha )           { _alpha = alpha;}
        void setFilename( const std::string& filename ) { _filename = filename;}

    private:
        eq::uint128_t _frameDataID;
        std::string   _windowSystem;
        uint32_t      _precision;
        float         _brightness;
        float         _alpha;
        std::string   _filename;
    };
}


#endif // EVOLVE_INITDATA_H

