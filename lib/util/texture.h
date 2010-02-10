
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/client/os.h>           // GLEW
#include <eq/client/frame.h>        // Frame::Buffer enum

#include <eq/base/thread.h>         // thread debug macro
#include <eq/base/nonCopyable.h>    // base class

namespace eq
{
    class Image;
    class PixelViewport;

namespace util
{
    /** 
     * A wrapper around GL textures.
     * 
     * So far used by the Image and Compositor. The target is assumed to be
     * GL_TEXTURE_RECTANGLE_ARB.
     */
    class Texture : public base::NonCopyable
    {
    public:
        /** Construct a new Texture. */
        EQ_EXPORT Texture( GLEWContext* const glewContext = 0 );
        EQ_EXPORT virtual ~Texture();

        /** Clear the texture, including the GL texture name. */
        EQ_EXPORT void flush();

        /** Set the target of the texture. */
        EQ_EXPORT void setTarget( const GLenum target );

        /** @return the target of the texture. */
        GLenum getTarget() const { return _target; }

        /** Set the internal pixel format of the texture, e.g., GL_RGBA16F. */
        EQ_EXPORT void setInternalFormat( const GLuint format );

        /** @return the pixel format of the texture. */
        GLuint getInternalFormat() const { return _internalFormat; }

        /** @return the data format of the texture, e.g., GL_RGBA. */
        GLuint getFormat() const { return _format; }

        /** @return the data type of the texture, e.g., GL_HALF_FLOAT. */
        GLuint getType() const { return _type; }

        /** 
         * Copy the specified area from the current read buffer to the
         * texture at 0,0.
         */
        EQ_EXPORT void copyFromFrameBuffer( const PixelViewport& pvp );

        /** Copy the specified image buffer to the texture at 0,0. */
        EQ_EXPORT void upload( const Image* image, const Frame::Buffer which );

        /** Copy the specified buffer to the texture at 0,0. */
        EQ_EXPORT void upload( const int width, const int height, void* ptr );

        /** Copy the texture data to the given memory address. */
        EQ_EXPORT void download( void* buffer, const uint32_t format, 
                                 const uint32_t type ) const;

        /**
         * Copy the texture data to the given memory address, using the internal
         * format and type.
         */
        EQ_EXPORT void download( void* buffer ) const;

        /** Bind the texture. */
        EQ_EXPORT void bind() const;

        /** Create and bind a texture to the current FBO. */
        EQ_EXPORT void bindToFBO( const GLenum target, const int width,
                                  const int height );

        /** Resize the texture. */
        EQ_EXPORT void resize( const int width, const int height );

        /** @return true if the texture can be bound. */
        EQ_EXPORT bool isValid() const;

        /** Writes the texture data as a rgb image file. */
        EQ_EXPORT void writeTexture( const std::string& filename,
                                     const eq::Frame::Buffer buffer,
                                     const PixelViewport& pvp ) const;

        GLEWContext* glewGetContext() { return _glewContext; }
        const GLEWContext* glewGetContext() const { return _glewContext; }
        void setGLEWContext( GLEWContext* context ) { _glewContext = context; }

    private:
        /** The GL texture name. */
        GLuint _id;

        /** the target of the texture. */
        GLenum _target;

        /** The GL pixel format (format+type). */
        GLuint _internalFormat;

        /** texture data format, complementary to pixel texture format */
        GLuint _format;

        /** texture data type */
        GLuint _type;

        /** The maximum width of the texture. */
        int32_t _width;

        /** The maximum height of the texture. */
        int32_t _height;

        /** false if the texture needs to be defined, true if not. */
        bool _defined;

        GLEWContext* _glewContext;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Generate, if needed, a GL texture name. */
        void _generate();

        /** Set the size of the texture, updating the _defined flag. */
        void _resize( const int32_t width, const int32_t height );

        CHECK_THREAD_DECLARE( _thread );
    };
}
}

#endif // EQUTIL_TEXTURE_H
