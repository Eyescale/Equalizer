
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef EQUTIL_ACCUM_H
#define EQUTIL_ACCUM_H

#include <eq/util/types.h>

namespace eq
{
namespace util
{
    /**
     * A C++ class to abstract an accumulation buffer.
     *
     * Depending on the OpenGL version, an FBO or glAccum() is used.
     *
     * Remark: MacOS systems seem to have a buggy implementation of glAccum(),
     * and use a workaround which needs setTotalSteps() to set up the total
     * number of accumulations done.
     */
    class Accum
    {
    public: 
        /** Construct a new accumulation buffer. @version 1.0 */
        EQ_API Accum( const GLEWContext* const glewContext );

        /** Destruct the accumulation buffer. @version 1.0 */
        EQ_API ~Accum();

        /**
         * Initialize the accumulation buffer.
         *
         * @param pvp the pixel viewport.
         * @param textureFormat the texture format.
         * @return true if initialized successfully.
         * @version 1.0
         */
        EQ_API bool init( const PixelViewport& pvp, unsigned textureFormat );

        /** Exit the accum buffer. @version 1.0 */
        EQ_API void exit();

        /**
         * Resize the accumulation buffer.
         *
         * @param width the new width.
         * @param height the new height.
         * @return true if the accumulation buffer is correctly resized.
         * @version 1.0
         */
        EQ_API bool resize( const int width, const int height );

        /**
         * Resize the accumulation buffer.
         *
         * @param pvp The pixel viewport where the accumulation buffer will
         *        be displayed
         * @return true if the accumulation buffer is correctly resized.
         * @version ??
         */
        EQ_API bool resize( const PixelViewport& pvp );

        /** Clear the accumulation buffer. @version 1.0 */
        EQ_API void clear();

        /**
         * Accumulate a frame from the read buffer into the accumulation buffer.
         * @version 1.0
         */
        EQ_API void accum();

        /**
         * Copy the result of the accumulation to the current draw buffer.
         * @version 1.0
         */
        EQ_API void display();

        /**
         * Get the maximum number of accumulation steps possible.
         *
         * @return the maximum number of steps.
         * @version 1.0
         */
        EQ_API uint32_t getMaxSteps() const;

        /**
         * Get the current number of accumulation steps done.
         *
         * @return the number of steps done.
         * @version 1.0
         */
        uint32_t getNumSteps() const { return _numSteps; }

        /**
         * Set the total number of accumulation steps that will be done.
         *
         * This is used only for Darwin systems due to a specific glAccum()
         * workaround.
         *
         * @param totalSteps the total number of steps to do.
         * @version 1.0
         */
        void setTotalSteps( uint32_t totalSteps )
            { _totalSteps = totalSteps; }

        /** @return the total number of accumulations. @version 1.0 */
        uint32_t getTotalSteps() { return _totalSteps; }

        /**
         * Test if the accumulation uses the FBO implementation.
         *
         * @return true if the accumulation uses an FBO, false if it uses
         *         glAccum().
         */
        EQ_API bool usesFBO() const;

        /**
         * @internal
         * Test if the accumulation would use the FBO implementation.
         *
         * @return true if the accumulation uses an FBO, false if it uses
         *         glAccum().
         */
        EQ_API static bool usesFBO( const GLEWContext* glewContext );

        const GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        const GLEWContext* const _glewContext;

        int _width;
        int _height;

        AccumBufferObject* _abo;
        uint32_t _numSteps;
        uint32_t _totalSteps;
    };
}
}

#endif // EQUTIL_ACCUM_H
