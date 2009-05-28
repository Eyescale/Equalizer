
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

namespace eqPly
{
    class View;
    typedef std::vector< View* > ViewVector;

    /**
     * Frame-specific data.
     *
     * The frame-specific data is used as a per-config distributed object and
     * contains mutable, rendering-relevant data. Each rendering thread (pipe)
     * keeps its own instance synchronized with the frame currently being
     * rendered. The data is managed by the Config, which modifies it directly.
     */
    class FrameData : public eq::Object
    {
    public:

        FrameData();

        virtual ~FrameData() {};

        void reset();
        
        /** @name Rendering flags. */
        //*{
        void setModelID( const uint32_t id );

        void setColor( const bool onOff );
        void setRenderMode( const mesh::RenderMode mode );

        void setTranslation(   const vmml::Vector3f& translation );
        void setRotation(      const vmml::Vector3f& rotation    );
        void setModelRotation( const vmml::Vector3f& rotation    );

        void toggleOrtho();
        void toggleStatistics();
        void toggleHelp();
        void toggleWireframe();
        void togglePilotMode();
        void toggleRenderMode();

        uint32_t getModelID() const { return _modelID; }
        bool useColor() const { return _color; }
        bool useOrtho() const { return _ortho; }
        bool useStatistics() const { return _statistics; }
        bool showHelp() const { return _help; }
        bool useWireframe() const { return _wireframe; }
        bool usePilotMode() const { return _pilotMode; }
        mesh::RenderMode getRenderMode() const { return _renderMode; }
        //*}

        /** @name Camera parameters. */
        //*{
        void spinCamera( const float x, const float y );
        void spinModel(  const float x, const float y );
        void moveCamera( const float x, const float y, const float z );
        void setCameraPosition( const float x, const float y, const float z );

        const vmml::Matrix4f& getCameraRotation() const
            { return _rotation; }
        const vmml::Matrix4f& getModelRotation() const
            { return _modelRotation; }
        const vmml::Vector3f& getCameraTranslation() const
            { return _translation; }
        //*}

        /** @name View interface. */
        //*{
        void setCurrentViewID( const uint32_t id );
        uint32_t getCurrentViewID() const { return _currentViewID; }
        //*}

    protected:
        /** @sa Object::serialize() */
        virtual void serialize( eq::net::DataOStream& os,
                                const uint64_t dirtyBits );
        /** @sa Object::deserialize() */
        virtual void deserialize( eq::net::DataIStream& is,
                                  const uint64_t dirtyBits );

        /** The changed parts of the data since the last pack(). */
        enum DirtyBits
        {
            DIRTY_CAMERA = eq::Object::DIRTY_CUSTOM << 0,
            DIRTY_FLAGS  = eq::Object::DIRTY_CUSTOM << 1,
            DIRTY_VIEW   = eq::Object::DIRTY_CUSTOM << 2,
        };

    private:
        vmml::Matrix4f _rotation;
        vmml::Matrix4f _modelRotation;
        vmml::Vector3f _translation;
        
        uint32_t         _modelID;
        mesh::RenderMode _renderMode;
        bool             _color;
        bool             _ortho;
        bool             _statistics;
        bool             _help;
        bool             _wireframe;
        bool             _pilotMode;

        uint32_t _currentViewID;
    };
}


#endif // EQ_PLY_FRAMEDATA_H

