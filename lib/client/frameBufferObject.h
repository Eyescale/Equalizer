/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
                 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_FRAMEBUFFEROBJECT_H 
#define EQ_FRAMEBUFFEROBJECT_H 

#include <eq/client/windowSystem.h> // for GL types
#include <eq/client/texture.h>      // member

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

        /** De-initialize the Frame Buffer Object. */
        void exit();
        
        /** Bind to the Frame Buffer Object */
        void bind();
        
        /** Unbind any Frame Buffer Object */
        void unbind();
        
        /** ask if FBO built construction is ok */
        bool checkFBOStatus() const;
       
        /* Resize FBO, if needed. */
        bool resize( const int width, const int height );
          
        GLEWContext* glewGetContext() { return _glewContext; }
        const GLEWContext* glewGetContext() const { return _glewContext; }

        /** @return the color texture. */
        const Texture& getColorTexture() const { return _color; }

        /** @return the depth texture. */
        const Texture& getDepthTexture() const { return _depth; }

        /** @return the stencil texture. */
        const Texture& getStencilTexture() const { return _stencil; }

    private:
        GLuint _fboID;
         
        int _width;
        int _height;
         
        Texture _color;
        Texture _depth;
        Texture _stencil;
        
        GLEWContext* const _glewContext;

        CHECK_THREAD_DECLARE( _thread );
    };
}


#endif //EQ_FRAMEBUFFEROBJECT_H 
