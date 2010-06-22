
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch>
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

#ifndef FOB_TRACKER_H
#define FOB_TRACKER_H

#include <eq/eq.h>
#include <string>

namespace eqPly
{
    class Tracker
    {
    public:
        /** 
         * Constructs a new Tracker and sets it's _running state to false.
         */
        Tracker();

        /**
         * Configures the serial port and initialises the tracker.
         *
         * Sets the _running state to true if the initialisation is successful.
         *
         * @param port the used serial port.
         * @return <code>true</code> if the tracker works correctly,
         *         <code>false</code> otherwise.
         */
        bool init( const std::string& port );

        /** 
         * Set the matrix defining the transformation from world coordinates to
         * emitter coordinates.
         *
         * @param matrix the world to emitter matrix.
         */
        void setWorldToEmitter( const eq::Matrix4f& matrix )
            { _worldToEmitter = matrix; }

        /** 
         * Set the matrix defining the transformation from sensor coordinates to
         * coordinates of the tracked object.
         *
         * @param matrix the sensor to object matrix.
         */
        void setSensorToObject( const eq::Matrix4f& matrix )
            { _sensorToObject = matrix; }

        /**
         * Checks if the tracker is running
         *
         * @return <code>true</code> if the tracker is ready,
         *         <code>false</code> otherwise.
         */
        bool isRunning() const { return _running; }

        /**
         * Gets new position and orientation data from the tracker and
         * stores them in the _pos and _hpr arrays.
         *
         * For a successful update, _running must be true.
         *
         * @return <code>true</code> if the data transfer is successful,
         *         <code>false</code> otherwise.
         */
        bool update();

        /**
         * Gets the transformation matrix with the position and orientation
         * data.
         *
         * This function will not communicate with the tracker,.
         *
         * @return the transformation matrix.
         */
        const eq::Matrix4f& getMatrix() const { return _matrix; }

    private:
        bool _update(); //update without state checking
        bool _read( unsigned char* buffer, const size_t size,
                    const unsigned long int timeout );

        /** The state defining if the tracker is running. */
        bool  _running;

        int   _fd;

        /**
         * Matrix defining the orientation and position of the tracked 
         * object.
         */
        eq::Matrix4f _matrix;

        /** world to emitter transformation */
        eq::Matrix4f _worldToEmitter;
        /** sensor to object transformation */
        eq::Matrix4f _sensorToObject;
    };
}

#endif // FOB_TRACKER_H
