
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
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

#ifndef EVOLVE_FRAMEDATA_H
#define EVOLVE_FRAMEDATA_H

#include "eVolve.h"

#include <eq/eq.h>

namespace eVolve
{
    class FrameData : public co::Serializable
    {
    public:

        FrameData();

        void reset();

        /** @name Rendering flags. */
        //*{
        void setOrtho( const bool ortho );
        void adjustQuality( const float delta );
        void toggleBackground();
        void toggleNormalsQuality();
        void toggleColorMode();
        void toggleOrtho();
        void toggleHelp();
        void toggleStatistics();

        void spinCamera( const float x, const float y );
        void moveCamera( const float x, const float y, const float z );

        void setTranslation( const eq::Vector3f& translation );
        void setRotation(    const eq::Vector3f& rotation    );

        bool showHelp()      const { return _help;       }
        bool useOrtho( )     const { return _ortho;      }
        bool useStatistics() const { return _statistics; }

        const eq::Vector3f& getTranslation() const { return _translation; }
        const eq::Matrix4f& getRotation()    const { return _rotation;    }
        float getQuality() const { return _quality; }
        ColorMode getColorMode() const { return _colorMode; }
        BackgroundMode getBackgroundMode() const { return _bgMode; }
        NormalsQuality getNormalsQuality() const { return _normalsQuality; }
        //*}

        /** @name View interface. */
        //*{
        void setCurrentViewID( const eq::uint128_t& id );

        eq::uint128_t getCurrentViewID() const { return _currentViewID; }
        //*}

        /** @name Message overlay. */
        //*{
        void setMessage( const std::string& message );
        const std::string& getMessage() const { return _message; }
        //*}

    protected:
        /** @sa Object::serialize() */
        virtual void serialize(         co::DataOStream& os,
                                  const uint64_t              dirtyBits );

        /** @sa Object::deserialize() */
        virtual void deserialize(       co::DataIStream& is,
                                  const uint64_t              dirtyBits );


        virtual ChangeType getChangeType() const { return DELTA; }

        /** The changed parts of the data since the last pack(). */
        enum DirtyBits
        {
            DIRTY_CAMERA  = co::Serializable::DIRTY_CUSTOM << 0,
            DIRTY_FLAGS   = co::Serializable::DIRTY_CUSTOM << 1,
            DIRTY_VIEW    = co::Serializable::DIRTY_CUSTOM << 2,
            DIRTY_MESSAGE = co::Serializable::DIRTY_CUSTOM << 3,
        };

    private:

        eq::Matrix4f    _rotation;
        eq::Vector3f    _translation;
        bool            _ortho;
        ColorMode       _colorMode;
        BackgroundMode  _bgMode;
        NormalsQuality  _normalsQuality;
        bool            _statistics;
        bool            _help;
        float           _quality;
        eq::uint128_t   _currentViewID;
        std::string     _message;
    };
}


#endif // EVOLVE_FRAMEDATA_H

