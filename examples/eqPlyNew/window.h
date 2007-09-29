
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_WINDOW_H
#define EQ_PLY_WINDOW_H

#include <eq/eq.h>

#include "vertexBufferState.h"

namespace eqPly
{
    class VertexBufferState : public mesh::EqVertexBufferState, 
                              public eqBase::Referenced
    {
    public:
        VertexBufferState( const eq::GLFunctions* glFunctions )
                : mesh::EqVertexBufferState( glFunctions ) {}
        virtual ~VertexBufferState() {}
    };
    
    class Window : public eq::Window
    {
    public:
        Window() : _logoTexture( 0 ) {}

        void getLogoTexture( GLuint& id, vmml::Vector2i& size ) const
            { id = _logoTexture; size = _logoSize; }
        
        mesh::VertexBufferState& getState() { return *_state; }
        
    protected:
        virtual ~Window() {}
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

    private:
        eqBase::RefPtr< VertexBufferState > _state;

        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
        void _loadShaders();
    };
}

#endif // EQ_PLY_WINDOW_H
