
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_TEXTURE_H
#define EQ_TEXTURE_H

#include <eq/client/windowSystem.h> // GL types
#include <eq/client/frame.h>        // Frame::Buffer enum

#include <eq/base/thread.h>         // thread debug macro
#include <eq/base/nonCopyable.h>    // base class

namespace eq
{
    class Image;
    class PixelViewport;

    /** 
     * A wrapper around GL textures.
     * 
     * So far used by the Image and Compositor. The target is assumed to be
     * GL_TEXTURE_RECTANGLE_ARB.
     */
    class Texture : public base::NonCopyable
    {
    public:
        /** Constructs a new Texture. */
        Texture();
        ~Texture();
     
        /** Clear the texture, including the GL texture name. */
        void flush();

        /** Set the internal format of the texture. */
        void setFormat( const GLuint format );

        /** Copy the specified area from the current read buffer to 0,0. */
        void copyFromFrameBuffer( const PixelViewport& pvp );

        /** Copy the specified image buffer to 0,0. */
        void upload( const Image* image, const Frame::Buffer which );

        /** @return the GL texture name. */
        GLuint getID() const { return _id; }

        /** Bind the texture. */
        void bind() const
            { EQASSERT( _id ); glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _id ); }

    private:
        /** The GL texture name. */
        GLuint _id;

        /** The GL internal texture format. */
        GLuint _format;

        /** The maximum width of the texture. */
        int32_t _width;

        /** The maximum height of the texture. */
        int32_t _height;

        /** false if the texture needs to be defined, true if not. */
        bool _defined;

        /** Generate, if needed, a GL texture name. */
        void _generate();

        /** Set the size of the texture, updating the _defined flag. */
        void _resize( const int32_t width, const int32_t height );

        CHECK_THREAD_DECLARE( _thread );
    };
};
#endif // EQ_TEXTURE_H
