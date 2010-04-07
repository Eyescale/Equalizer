
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef EQUTIL_ACCUMBUFFEROBJECT_H
#define EQUTIL_ACCUMBUFFEROBJECT_H

#include <eq/util/frameBufferObject.h> // base class

#include <eq/fabric/pixelViewport.h> // member

namespace eq
{
namespace util
{
    class Texture;

    /** 
     * A class to emulate an OpenGL accumulation buffer using an FBO. 
     * @sa glAccum()
     */
    class AccumBufferObject : public FrameBufferObject
    {
    public: 
        /** Construct a new Accumulation Buffer Object */
        EQ_EXPORT AccumBufferObject( GLEWContext* const glewContext );

        /** Destruct the Accumulation Buffer Object */
        EQ_EXPORT ~AccumBufferObject();

        /**
         * Initialize the Accumulation Buffer Object.
         *
         * The ABO uses a 32-bit float texture.
         * 
         * @param pvp the initial pixel viewport of the rendering buffer.
         * @param format the texture format corresponding to the source color
         *               read buffer.
         * @return true on success, false otherwise
         * @sa Window::getColorFormat(), glReadBuffer()
         */
        EQ_EXPORT bool init( const PixelViewport& pvp, const GLuint format );

        /** De-initialize the Accumulation Buffer Object. */
        EQ_EXPORT void exit();

        /**
         * Load the current read buffer into the accumulation buffer.
         * The values of the current read buffer are multiplied with value and
         * copied into the accumulation buffer.
         *
         * @param value a floating-point value multiplying the source values
         *              during the load operation.
         */
        EQ_EXPORT void load( const GLfloat value );

        /**
         * Accumulate the current read buffer into the accumulation buffer.
         * The read buffer is multiplied by value and added to the accumulation
         * buffer.
         *
         * @param value a floating-point value multiplying the source values
         *              during the accum operation.
         */
        EQ_EXPORT void accum( const GLfloat value );

        /**
         * Transfers accumulation buffer values to the write buffer.
         * Each accumulation buffer is multiplied by value during the transfer.
         *
         * @param value a floating-point value multiplying the accumulation
         *              values during the operation.
         */
        EQ_EXPORT void display( const GLfloat value );

    private:
        /**
         * Draw a textured quad.
         *
         * @param texture a texture object.
         * @param value the brightness factor of the result.
         */
        void _drawQuadWithTexture( Texture* texture, const PixelViewport& pvp,
                                   const GLfloat value );

        Texture* _texture;
        PixelViewport _pvp;
    };
}
}

#endif // EQUTIL_ACCUMBUFFEROBJECT_H
