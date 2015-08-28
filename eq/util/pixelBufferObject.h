
/* Copyright (c)      2012, Maxim Makhinya <maxmah@gmail.com>
 *               2012-2014, Stefan Eilemann <eile@eyescale.ch>
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

#include <lunchbox/lock.h>

#include <eq/api.h>
#include <eq/error.h> // enum
#include <eq/types.h>

namespace eq
{
namespace util
{
namespace detail { class PixelBufferObject; }

/** A C++ class to abstract OpenGL pixel buffer objects.
 *
 * If multiple PBOs of the same read/write type are used in the same glContext
 * they should be bound/mapped and unbound/unmapped sequentially.
 *
 * If thread-safe mode is used, buffer binding and mapping is locked until the
 * corresponding unbind/unmap happened.
 *
 * On correct PBO usage see: http://www.songho.ca/opengl/gl_pbo.html
 */
class PixelBufferObject
{
public:
    /**
     * Construct a new pixel buffer object.
     *
     * @param glewContext the OpenGL function table.
     * @param threadSafe true if PBO shall use locks to synchronize access.
     * @version 1.3
     */
    EQ_API PixelBufferObject( const GLEWContext* glewContext,
                              const bool threadSafe );

    /** Destruct the pixel buffer object */
    EQ_API virtual ~PixelBufferObject();

    /**
     * Initialize the pixel buffer object.
     *
     * The PBO is bound after a successful operation.
     *
     * @param size total number of bytes (not 0)
     * @param type the access type: GL_READ_ONLY_ARB or GL_WRITE_ONLY_ARB
     * @return ERROR_NONE on success, the Error code on failure.
     * @version 1.3
     */
    EQ_API virtual Error setup( const size_t size, const unsigned type );

    /** Unbind and de-initialize the pixel buffer object. @version 1.3 */
    EQ_API virtual void destroy();

    /**
     * Bind the PBO and map its data for reading.
     *
     * @return pointer to the PBO memory
     * @version 1.3
     */
    EQ_API virtual const void* mapRead() const;

    /**
     * Bind the PBO and map its data for writing.
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
    EQ_API size_t getSize() const;

    /** @return true if the pbo is intialized. @version 1.3 */
    EQ_API bool isInitialized() const;

    /** @return true if the access to pbo is blocking. @version 1.3 */
    EQ_API bool isThreadSafe() const;

    /** @return OpenGL ID @version 1.3.2 */
    unsigned getID() const;

private:
    PixelBufferObject( const PixelBufferObject& ) = delete;
    PixelBufferObject& operator=( const PixelBufferObject& ) = delete;
    detail::PixelBufferObject* const _impl;
};
}
}

#endif // EQUTIL_PIXELBUFFEROBJECT_H
