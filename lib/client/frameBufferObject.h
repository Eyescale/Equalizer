/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
   All rights reserved. */

#ifndef EQ_FRAMEBUFFEROBJECT_H 
#define EQ_FRAMEBUFFEROBJECT_H 

#include <eq/client/windowSystem.h> // for GL types

namespace eq
{
    /** A C++ class to abstract GL frame buffer objects */
    class FrameBufferObject 
    {
    public: 
        
        
        /** Constructs a new Frame Buffer Object */
		FrameBufferObject( GLEWContext* const glewContext );
        
        /** Destruct the Frame Buffer Object */
		~FrameBufferObject();
        
        /** Initialize the Frame Buffer Object */
        bool init( const int width, const int height, 
                   const int depthSize, const int stencilSize );
        
        /** Bind to the Frame Buffer Object */
        void bind();
        
        /** ask if FBO built construction is ok */
        bool checkFBOStatus();
        
        /** 
         * Get the GLEW context for this window.
         * 
         * The glew context is initialized during window initialization, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any supported GL function can be called directly
         * from an initialized Window.
         * 
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         */
        GLEWContext* glewGetContext() { return _glewContext; }
    private:
        GLuint _fboID;
         
        enum iTextureType
        {
            COLOR_TEXTURE,
            DEPTH_TEXTURE,
            STENCIL_TEXTURE,
            ALL_TEXTURE
        };
        
        GLuint _textureID[ALL_TEXTURE];
        
        GLEWContext* const   _glewContext;
    };
}


#endif //EQ_FRAMEBUFFEROBJECT_H 