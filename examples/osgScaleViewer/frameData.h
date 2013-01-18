
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef OSGSV_FRAMEDATA_H
#define OSGSV_FRAMEDATA_H

#include <eq/eq.h>

#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

namespace osgScaleViewer
{
    class FrameData : public co::Serializable
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
        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );

        /** @sa Object::deserialize() */
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

        /** The changed parts of the data since the last pack(). */
        enum DirtyBits
        {
            DIRTY_CAMERA  = co::Serializable::DIRTY_CUSTOM << 0,
            DIRTY_FLAGS   = co::Serializable::DIRTY_CUSTOM << 1
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
