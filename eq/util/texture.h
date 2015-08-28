
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQUTIL_TEXTURE_H
#define EQUTIL_TEXTURE_H

#include <eq/frame.h> // Frame::Buffer enum
#include <lunchbox/thread.h> // thread debug macro

namespace eq
{
namespace util
{
namespace detail { class Texture; }

/**
 * A wrapper around OpenGL textures.
 *
 * So far used by the Image and Compositor. The target is assumed to be
 * GL_TEXTURE_RECTANGLE_ARB or GL_TEXTURE_2D.
 */
class Texture
{
public:
    /**
     * Construct a new Texture.
     *
     * A GLEWContext might be provided later using setGLEWContext(). It is
     * needed for operations using OpenGL extensions or version 1.2 or later
     * functions.
     * @version 1.0
     */
    EQ_API Texture( const unsigned target,
                    const GLEWContext* const glewContext = 0 );

    /** Destruct the texture. @version 1.0 */
    EQ_API virtual ~Texture();

    /** @name Data Access. */
    //@{
    /** @return the target of the texture. @version 1.0 */
    EQ_API unsigned getTarget() const;

    /**
     * @internal
     * @return the compressor target corresponding to the texture target.
     */
    uint32_t getCompressorTarget() const;

    /**
     * @return the internal (GPU) pixel format of the texture.
     * @sa init()
     * @version 1.0
     */
    EQ_API unsigned getInternalFormat() const;

    /**
     * Set the external data format and type.
     *
     * @param format the OpenGL format.
     * @param type the OpenGL Type.
     * @version 1.0
     */
    EQ_API void setExternalFormat( const uint32_t format,
                                   const uint32_t type );

    /**
     * @return the external data format of the texture, e.g., GL_RGBA.
     * @version 1.0
     */
    EQ_API unsigned getFormat() const;

    /**
     * @return the external data type of the texture, e.g., GL_HALF_FLOAT.
     * @version 1.0
     */
    EQ_API unsigned getType() const;

    /** @return the OpenGL texture name. @version 1.0 */
    EQ_API unsigned getName() const;

    /** @return the current width. @version 1.0 */
    EQ_API int32_t getWidth() const;

    /** @return the current height. @version 1.0 */
    EQ_API int32_t getHeight() const;

    /** @return true if the texture can be bound. @version 1.0 */
    EQ_API bool isValid() const;
    //@}

    /** @name Operations. */
    //@{
    /**
     * Initialize an OpenGL texture.
     *
     * @param internalFormat the OpenGL texture internal format.
     * @param width the width of the texture.
     * @param height the height of the texture.
     * @version 1.0
     */
    EQ_API void init( const unsigned internalFormat, const int32_t width,
                      const int32_t height );

    /**
     * Clear the texture, including deleting the GL texture name.
     * @version 1.0
     */
    EQ_API void flush();

    /** @internal Apply the given texture filtering parameter. */
    EQ_API void applyZoomFilter( const ZoomFilter filter ) const;

    EQ_API void applyWrap() const; //<! @internal

    /**
     * Copy the specified area from the current read buffer to the
     * texture at 0,0.
     * @version 1.0
     */
    EQ_API void copyFromFrameBuffer( const unsigned internalFormat,
                                     const fabric::PixelViewport& pvp );

    /** Copy the specified buffer to the texture at 0,0. @version 1.0 */
    EQ_API void upload( const int32_t width, const int32_t height,
                        const void* ptr );

    /**
     * Copy the texture data from the GPU to the given memory address.
     * @version 1.0
     */
    EQ_API void download( void* buffer ) const;

    /** Bind the texture. @version 1.0 */
    EQ_API void bind() const;

    /** Create and bind a texture to the current FBO. @version 1.0 */
    EQ_API void bindToFBO( const unsigned target, const int32_t width,
                           const int32_t height, const int32_t samples = 1 );

    /** Resize the texture. @version 1.0 */
    EQ_API void resize( const int32_t width, const int32_t height );

    /** Write the texture data as an rgb image file. @version 1.0 */
    EQ_API void writeRGB( const std::string& filename ) const;
    //@}

    EQ_API const GLEWContext* glewGetContext() const;
    EQ_API void setGLEWContext( const GLEWContext* context );

    /** @name Wrap existing GL textures */
    //@{
    /**
     * Flush the texture without deleting the GL texture name.
     * @version 1.0
     */
    EQ_API void flushNoDelete();

    /**
     * Use an OpenGL texture created externally.
     *
     * The previous GL texture, if any, is deallocated using flush(). The new
     * texture has to be of the correct target, size and internal format. The
     * texture is validated by this method.
     *
     * @param id The OpenGL texture name.
     * @param internalFormat the OpenGL texture internal format.
     * @param width the width of the texture.
     * @param height the height of the texture.
     * @version 1.0
     */
    EQ_API void setGLData( const unsigned id, const unsigned internalFormat,
                           const int32_t width, const int32_t height );
    //@}

private:
    Texture( const Texture& ) = delete;
    Texture& operator=( const Texture& ) = delete;
    detail::Texture* const _impl;

    /**
     * Set the internal pixel format of the texture, e.g., GL_RGBA16F.
     *
     * Automatically sets the external format and type to one matching the
     * internal format.
     *
     * @param internalFormat the OpenGL internal texture format.
     */
    void _setInternalFormat( const unsigned internalFormat );

    /** Generate, if needed, a GL texture name. */
    void _generate();

    /** Set the size of the texture, updating the _defined flag. */
    void _grow( const int32_t width, const int32_t height );

    LB_TS_VAR( _thread );
};
/** Print the texture state to the given output stream. @version 1.7.1 */
EQ_API std::ostream& operator << ( std::ostream&, const Texture& );

}
}

#endif // EQUTIL_TEXTURE_H
