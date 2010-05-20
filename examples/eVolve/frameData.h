
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *               2007-2009, Maxim Makhinya
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#ifndef EVOLVE_FRAMEDATA_H
#define EVOLVE_FRAMEDATA_H

#include "eVolve.h"

#include <eq/eq.h>

namespace eVolve
{
    class FrameData : public eq::Object
    {
    public:

        FrameData();

        void reset();

        /** @name Rendering flags. */
        //*{
        void setOrtho( const bool ortho );
        void adjustQuality( const float delta );
        void toggleOrtho( );
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
        //*}

        /** @name View interface. */
        //*{
        void setCurrentViewID( const uint32_t id );

        uint32_t getCurrentViewID() const { return _currentViewID; }
        //*}

        /** @name Message overlay. */
        //*{
        void setMessage( const std::string& message );
        const std::string& getMessage() const { return _message; }
        //*}

    protected:
        /** @sa Object::serialize() */
        virtual void serialize(         eq::net::DataOStream& os,
                                  const uint64_t              dirtyBits );

        /** @sa Object::deserialize() */
        virtual void deserialize(       eq::net::DataIStream& is,
                                  const uint64_t              dirtyBits );


        virtual ChangeType getChangeType() const { return DELTA; }

        /** The changed parts of the data since the last pack(). */
        enum DirtyBits
        {
            DIRTY_CAMERA  = eq::Object::DIRTY_CUSTOM << 0,
            DIRTY_FLAGS   = eq::Object::DIRTY_CUSTOM << 1,
            DIRTY_VIEW    = eq::Object::DIRTY_CUSTOM << 2,
            DIRTY_MESSAGE = eq::Object::DIRTY_CUSTOM << 3,
        };

    private:

        eq::Matrix4f _rotation;
        eq::Vector3f _translation;
        bool         _ortho;
        bool         _statistics;
        bool         _help;
        float        _quality;
        uint32_t     _currentViewID;
        std::string  _message;
    };
}


#endif // EVOLVE_FRAMEDATA_H

