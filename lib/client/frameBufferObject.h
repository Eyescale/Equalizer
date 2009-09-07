
/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
                 2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_FRAMEBUFFEROBJECT_H 
#define EQ_FRAMEBUFFEROBJECT_H 

#include <eq/client/windowSystem.h> // for GL types
#include <eq/client/texture.h>      // member

#include <vector>

namespace eq
{
    /** A C++ class to abstract OpenGL frame buffer objects */
    class FrameBufferObject 
    {
    public: 
        /** Construct a new Frame Buffer Object */
        EQ_EXPORT FrameBufferObject( GLEWContext* const glewContext );

        /** Destruct the Frame Buffer Object */
        EQ_EXPORT ~FrameBufferObject();

        /**
         * Set format for all color textures, if desired format differs from the
         * default format GL_RGBA. This function should be called before init().
         *
         * @param format new format of color texture
         */
        EQ_EXPORT void setColorFormat( const GLuint format );

        /**
         * Add one color texture to the FBO.
         * 
         * One color texture is automatically created in the constructor. The
         * maximum number of textures per FBO is 16. Added color textures will
         * have the same format as the existing texture(s). This function has to
         * be called on an uninitialized FBO.
         *
         * @return false if color texture can't be added, otherwise true
         */
        EQ_EXPORT bool addColorTexture();

        /**
         * Initialize the Frame Buffer Object
         * 
         * @param width the initial width of the rendering buffer.
         * @param height the initial height of the rendering buffer.
         * @param depthSize The bit depth of the depth attachment
         * @param stencilSize The bit depth of the stencil attachment.
         * @return true on success, false otherwise
         * @sa resize(), getErrorMessage()
         */
        EQ_EXPORT bool init( const int width, const int height, 
                             const int depthSize, const int stencilSize );

        /** De-initialize the Frame Buffer Object. */
        EQ_EXPORT void exit();

        /**
         * Bind to the Frame Buffer Object as the read and draw buffer of the
         * current context.
         * @sa getErrorMessage()
         */
        EQ_EXPORT void bind();

        /**
         * Unbind any Frame Buffer Object and use the default drawable for the
         * current context.
         */
        EQ_EXPORT void unbind();

        /**
         * Resize the FBO.
         * 
         * The FBO has to be initialized and bound. It is not changed if the
         * size does not change.
         * @return true on success, false on error.
         * @sa getErrorMessage()
         */
        EQ_EXPORT bool resize( const int width, const int height );

        /** @return the color textures. */
        const TextureVector& getColorTextures() const { return _colors; }

        /** @return the depth texture. */
        const Texture& getDepthTexture() const { return _depth; }

        /** @return the stencil texture. */
        const Texture& getStencilTexture() const { return _stencil; }

        /** @return the reason for the last failed operation. */
        const std::string& getErrorMessage() { return _error; }

        /** @return the size of this framebuffer object. */
        EQ_EXPORT PixelViewport getPixelViewport() const;

        GLEWContext* glewGetContext() { return _glewContext; }
        const GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        GLuint _fboID;

        int _width;
        int _height;

        TextureVector _colors; //!< Multiple color textures

        Texture _depth;
        Texture _stencil;

        GLEWContext* const _glewContext;

        /** The reason for the last error. */
        std::string _error;

        bool _valid;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        CHECK_THREAD_DECLARE( _thread );

        /** Check the result after changes to an FBO. */
        bool _checkFBOStatus();
    };
}


#endif //EQ_FRAMEBUFFEROBJECT_H 
