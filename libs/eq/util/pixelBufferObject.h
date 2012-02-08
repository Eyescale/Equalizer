
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
 *               2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQUTIL_PIXELBUFFEROBJECT_H
#define EQUTIL_PIXELBUFFEROBJECT_H

#include <co/base/lock.h>

#include <eq/client/gl.h>   // for GLEW
#include <co/base/error.h>  // member

namespace eq
{
namespace util
{
namespace detail { class PixelBufferObject; }

    /** A C++ class to abstract OpenGL pixel buffer objects.
      *
      * If multiple PBOs of the same read/write type are used in the same 
      * glContext they should be binded/mapped and unbinded/unmapped 
      * sequentially.
      *
      * If thread-safe mode is used, buffer binding and mapping is locked until
      * the corresponding unbind/unmap happened.
      *
      * On correct PBO usage see: http://www.songho.ca/opengl/gl_pbo.html
      */
    class PixelBufferObject 
    {
    public:
        /**
         * Construct a new Pixel Buffer Object.
         *
         * @param glewContext the OpenGL function table.
         * @param threadSafe true if PBO shall use locks to synchronize access.
         * @version 1.3
         */
        EQ_API PixelBufferObject( const GLEWContext* glewContext,
                                  const bool threadSafe );

        /** Destruct the Frame Buffer Object */
        EQ_API virtual ~PixelBufferObject();

        /**
         * Initialize the Pixel Buffer Object.
         *
         * @param size total number of bytes (has to be > 0)
         * @param type the access type: GL_READ_ONLY_ARB or GL_WRITE_ONLY_ARB
         * @return true on success, false otherwise
         * @version 1.3
         */
        EQ_API virtual bool setup( const ssize_t size, const GLuint type );

        /** De-initialize the pixel buffer object. @version 1.3 */
        EQ_API virtual void destroy();

        /**
         * Bind the PBO and map its data for reading.
         *
         * @return pointer to the PBO memory
         * @version 1.3
         */
        EQ_API virtual const void* mapRead() const;

        /**
         * Bind the PBO and mappe its data for writing.
         *
         * @return pointer to the PBO memory
         * @version 1.3
         */
        EQ_API virtual void* mapWrite();

        /**
         * Unmap and unbind the PBO.
         *
         * @return pointer to the PBO memory
         * @version 1.3
         */
        EQ_API virtual void unmap() const;

        /** Bind the PBO. @version 1.3 */
        EQ_API virtual bool bind() const;

        /** Unbind the PBO. @version 1.3 */
        EQ_API virtual void unbind() const;

        /** @return the allocated size of the PBO. @version 1.3 */
        EQ_API ssize_t getSize() const;

        /** @return the reason for the last failed operation. @version 1.3 */
        EQ_API const co::base::Error& getError() const;

        /** @return true if the pbo is intialized. @version 1.3 */
        EQ_API bool isInitialized() const;

        /** @return true if the access to pbo is blocking. @version 1.3 */
        EQ_API bool isThreadSafe() const;

    private:
        detail::PixelBufferObject* const _impl;
    };
}
}

#endif // EQUTIL_PIXELBUFFEROBJECT_H 
