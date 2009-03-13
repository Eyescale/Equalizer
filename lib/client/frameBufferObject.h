
/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
                 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#ifndef EQ_FRAMEBUFFEROBJECT_H 
#define EQ_FRAMEBUFFEROBJECT_H 

#include <eq/client/windowSystem.h> // for GL types
#include <eq/client/texture.h>      // member

#include <vector>

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

        /**
         * Set format for all color textures, if desired format differs 
         * from default. This function should be called before init function.
         *
         * @param format new format of color texture
         */
        void setColorFormat( const GLuint format );

        /** De-initialize the Frame Buffer Object. */
        void exit();

        /** Bind to the Frame Buffer Object */
        void bind();

        /** Unbind any Frame Buffer Object */
        void unbind();

        /* Resize FBO, if needed. */
        bool resize( const int width, const int height );

        GLEWContext* glewGetContext() { return _glewContext; }
        const GLEWContext* glewGetContext() const { return _glewContext; }

        /** @return the color texture. */
        const Texture& getColorTexture( const uint8_t index = 0 ) const;

        /** @return the depth texture. */
        const Texture& getDepthTexture() const { return _depth; }

        /** @return the stencil texture. */
        const Texture& getStencilTexture() const { return _stencil; }

        /** @return the reason for the last failed operation. */
        const std::string& getErrorMessage() { return _error; }

        /** @return number of color textures that will be used in FBO. */
        inline uint8_t getNumberOfColorTextures() const { return _color.size();}

        /**
         * Add one texture to FBO. When FBO class is created it has one color
         * texture by default. Maximal number of textures per FBO is 16. Added
         * color texture will have the same format as already added textures.
         *
         * @return false if color texture can't be added, otherwise true
         */
        bool addColorTexture( );

    private:
        GLuint _fboID;

        int _width;
        int _height;

        uint8_t _resizedColorTextures;

        std::vector<Texture*> _color; //!< Multiple color textures
        GLuint _colorFormat;          //!< Format of color textures

        Texture _depth;
        Texture _stencil;

        GLEWContext* const _glewContext;

        /** The reason for the last error. */
        std::string _error;

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
