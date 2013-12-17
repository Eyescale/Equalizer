
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
 *   2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef OSGSV_CONFIG_H
#define OSGSV_CONFIG_H

#include "frameData.h"
#include "initData.h"
#include "tracker.h"

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

        bool loadInitData( const eq::uint128_t& initDataID );

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
        lunchbox::Clock _clock;
    };
}
#endif
