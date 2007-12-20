
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_WINDOW_H
#define EQ_PLY_WINDOW_H

#include <eq/eq.h>

#include "vertexBufferState.h"
#include <string>

namespace eqPly
{
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) 
                : eq::Window( parent ), _state( 0 ), _logoTexture( 0 ) {}

        void getLogoTexture( GLuint& id, vmml::Vector2i& size ) const
            { id = _logoTexture; size = _logoSize; }
        
        VertexBufferState& getState() { return *_state; }
        
    protected:
        virtual ~Window() {}
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

    private:
        VertexBufferState* _state;

        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
        void _loadShaders();
    };
}

#endif // EQ_PLY_WINDOW_H
