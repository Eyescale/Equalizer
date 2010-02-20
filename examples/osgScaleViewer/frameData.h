
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef OSGSV_FRAMEDATA_H
#define OSGSV_FRAMEDATA_H

#include <eq/eq.h>

#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

namespace osgScaleViewer
{
    class FrameData : public eq::Object
    {
    public:
        FrameData();

        /** 
         * Sets the camera position.
         * @param cameraPosition the camera position.
         */
        void setCameraPosition( eq::Vector3f cameraPosition );

        /** 
         * Gets the camera position.
         * @return the camera position.
         */
        const eq::Vector3f getCameraPosition() const { return _cameraPosition; }

        /** 
         * Sets the camera look at point.
         * @param cameraLookAtPoint the camera look at point vector.
         */
        void setCameraLookAtPoint( eq::Vector3f cameraLookAtPoint );

        /** 
         * Gets the camera look at point.
         * @return the camera look at point vector.
         */
        const eq::Vector3f getCameraLookAtPoint() const 
            { return _cameraLookAtPoint; }

        /** 
         * Sets the camera up vector.
         * @param cameraUpVector the camera up vector.
         */
        void setCameraUpVector( eq::Vector3f cameraUpVector );

        /** 
         * Gets the camera up vector.
         * @return the camera up vector.
         */
        const eq::Vector3f getCameraUpVector() const { return _cameraUpVector; }

        /** Toggle the display of run-time statistics. */
        void toggleStatistics();

        /** @return true of the statistics overlay is enabled. */
        bool useStatistics() const { return _statistics; }

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
            DIRTY_CAMERA  = eq::Object::DIRTY_CUSTOM << 0,
            DIRTY_FLAGS   = eq::Object::DIRTY_CUSTOM << 1
        };

        /** Reimplemented */
        virtual ChangeType getChangeType() const;

    private:
        /** Camera position, in world coordinates. */
        eq::Vector3f _cameraPosition;

        /** Point the camera is looking at, in world coordinates. */
        eq::Vector3f _cameraLookAtPoint;

        /** Up vector of the camera */
        eq::Vector3f _cameraUpVector;    

        bool _statistics;
    };
}
#endif
