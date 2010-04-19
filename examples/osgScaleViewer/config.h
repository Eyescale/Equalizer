
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

#ifndef OSGSV_CONFIG_H
#define OSGSV_CONFIG_H

#include "frameData.h"
#include "initData.h"
#include "tracker.h"

#define EQ_IGNORE_GLEW
#include <eq/eq.h>

namespace osgScaleViewer
{
    class Config : public eq::Config
    {
    public:

        Config( eq::ServerPtr parent );

        /** Reimplemented */
        virtual bool init();

        /** Reimplemented */
        virtual bool exit();

        /** Reimplemented */
        virtual uint32_t startFrame();

        /**
         * Reimplemented for camera controls.
         * If a keypress happens, this function updates _moveDirection, so
         * that the new camera position can be calculated in updateFrameData().
         * If a mouse move event happens, this function updates _pointerXDiff
         * and mPointerYDiff, so that the new camera viewing direction can be
         * calculated in updateFrameData().
         */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        /** 
         * Sets the InitData object.
         * @param data the init data.
         */
        void setInitData( const InitData& data );

        /** 
         * Gets the InitData object.
         * @return the init data.
         */
        const InitData& getInitData() const;

        /** Reimplemented */
        bool mapData( const uint32_t initDataID );

    protected:
        void updateFrameData( float elapsed );

    private:
        /** 
         * Sets the head matrix.
         * @param matrix the head matrix.
         */
        void _setHeadMatrix( const eq::Matrix4f& matrix );

        /** 
         * Gets the head matrix.
         * @return the head matrix.
         */
        const eq::Matrix4f& _getHeadMatrix() const;

        /** 
         * The vector of the camera movement in the current frame.
         * This value is computed in handleEvent() and then later in
         * updateFrameData(), the camera position is updated based on this.
         */
        eq::Vector3f _moveDirection;

        /** 
         * Same as the camera viewing direction, only that the y component
         * is always zero. Used in handleEvent() to calculate mMoveDirection.
         */
        eq::Vector3f _cameraWalkingVector;

        /** 
         * The differences of the mouse pointer position of this and
         * the last frame, in pixels. Used to calculate the new viewing direction.
         */
        int32_t _pointerXDiff;
        int32_t _pointerYDiff;

        /** 
         * The current angles of the camera.
         * The current camera viewing direction is calculated based on this.
         */
        float _cameraAngleHor;
        float _cameraAngleVer;

        /** Distributed objects. */
        InitData _initData;
        FrameData _frameData;

        /** Tracker. */
        Tracker _tracker;

        /** 
         * Clock used to measure the amount of time the last frame took,
         * to make it possible to have a framerate-independent rotation.
         */
        eq::base::Clock _clock;
    };
}
#endif
