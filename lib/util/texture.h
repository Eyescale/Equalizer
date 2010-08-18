
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

namespace util
{
    /** 
     * A wrapper around OpenGL textures.
     * 
     * So far used by the Image and Compositor. The target is assumed to be
     * GL_TEXTURE_RECTANGLE_ARB or GL_TEXTURE_2D.
     */
    class Texture : public base::NonCopyable
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
        EQ_EXPORT Texture( const GLenum target,
                           const GLEWContext* const glewContext = 0 );

        /** Destruct the texture. @version 1.0 */
        EQ_EXPORT virtual ~Texture();

        /** @name Data Access. */
        //@{
        /** @return the target of the texture. @version 1.0 */
        GLenum getTarget() const { return _target; }

        /** 
         * Set the external data format and type.
         *
         * @param format the OpenGL format.
         * @param type the OpenGl Type.
         */
        void setExternalFormat( const uint32_t format,
                                const uint32_t type );

        /** @return the pixel format of the texture. */
        GLuint getInternalFormat() const { return _internalFormat; }

        /** @return the data format of the texture, e.g., GL_RGBA. */
        GLuint getFormat() const { return _format; }

        /** @return the data type of the texture, e.g., GL_HALF_FLOAT. */
        GLuint getType() const { return _type; }

        /** @return the texture ID, */
        GLuint getID() const { return _id; }

        /** @return the current width */
        int32_t getWidth() const { return _width; }

        /** @return the current height */
        int32_t getHeight() const { return _height; }

        /** @return true if the texture can be bound. */
        EQ_EXPORT bool isValid() const;
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
        EQ_EXPORT void init( const GLuint internalFormat, const int32_t width,
                             const int32_t height );

        /** Clear the texture, including the GL texture name. @version 1.0 */
        EQ_EXPORT void flush();

        /**
         * Copy the specified area from the current read buffer to the
         * texture at 0,0.
         */
        EQ_EXPORT void copyFromFrameBuffer( const GLuint internalFormat,
                                            const fabric::PixelViewport& pvp );

        /**
         * Copy the specified image buffer to the texture at 0,0.
         */
        EQ_EXPORT void upload( const Image* image, const Frame::Buffer which,
                               ObjectManager< const void* >* glObjects );

        /** Copy the specified buffer to the texture at 0,0. */
        EQ_EXPORT void upload( const int32_t width, const int32_t height,
                               const void* ptr );

        /** Copy the texture data to the given memory address. */
        EQ_EXPORT void download( void* buffer, const uint32_t format, 
                                 const uint32_t type ) const;

        /** set a downloader for use during transfer operation  */
        void setDownloader( const uint32_t downloaderName )
            { _downloaderName = downloaderName; }

        /**
         * Copy the texture data to the given memory address, using the internal
         * format and type.
         */
        EQ_EXPORT void download( void* buffer ) const;

        /** Bind the texture. */
        EQ_EXPORT void bind() const;

        /** Create and bind a texture to the current FBO. */
        EQ_EXPORT void bindToFBO( const GLenum target, const int32_t width,
                                  const int32_t height );
        
        /** Resize the texture. */
        EQ_EXPORT void resize( const int32_t width, const int32_t height );

        /** Writes the texture data as a rgb image file. */
        EQ_EXPORT void writeRGB( const std::string& filename,
                                 const eq::Frame::Buffer buffer,
                                 const PixelViewport& pvp ) const;
        //@}

        const GLEWContext* glewGetContext() const { return _glewContext; }
        void setGLEWContext( const GLEWContext* context )
            { _glewContext = context; }

        /** @name Wrap existing GL textures */
        //@{
        /**
         * Flush the texture without deleting the GL texture name.
         * @version 1.0
         */
        EQ_EXPORT void flushNoDelete();

        /** 
         * Use an OpenGL texture created externally.
         *
         * The previous GL texture, if any, is deallocated using flush(). The
         * new texture has to be of the correct target, size and internal
         * format. The texture is validated by this method.
         *
         * @param id The OpenGL texture name.
         * @param internalFormat the OpenGL texture internal format.
         * @param width the width of the texture.
         * @param height the height of the texture.
         * @version 1.0
         */
        EQ_EXPORT void setGLData( const GLuint id, const GLuint internalFormat,
                                  const int32_t width, const int32_t height );
        //@}

    private:
        /** The GL texture name. */
        GLuint _id;

        /** the target of the texture. */
        const GLenum _target;

        /** The GL pixel format (format+type). */
        GLuint _internalFormat;

        /** texture data format, complementary to pixel texture format */
        GLuint _format;

        /** texture data type */
        GLuint _type;
        
        /** name of the default downloader */
        uint32_t _downloaderName;

        /** The maximum width of the texture. */
        int32_t _width;

        /** The maximum height of the texture. */
        int32_t _height;

        /** false if the texture needs to be defined, true if not. */
        bool _defined;

        const GLEWContext* _glewContext;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /**
         * Set the internal pixel format of the texture, e.g., GL_RGBA16F.
         *
         * Automatically sets the external format and type to one matching
         * the internal format.
         * 
         * @param internalFormat the OpenGL internal texture format.
         */
        void _setInternalFormat( const GLuint internalFormat );

        /** Generate, if needed, a GL texture name. */
        void _generate();
        
        /** Set the size of the texture, updating the _defined flag. */
        void _grow( const int32_t width, const int32_t height );

        EQ_TS_VAR( _thread );
    };
}
}

#endif // EQUTIL_TEXTURE_H
