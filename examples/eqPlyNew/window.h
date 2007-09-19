
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_WINDOW_H
#define EQ_PLY_WINDOW_H

#include <eq/eq.h>

#include "vertexBufferState.h"

namespace eqPly
{
    class ObjectManager : public eq::ObjectManager< const void* >, 
                          public eqBase::Referenced
    {
    public:
        ObjectManager( const eq::GLFunctions* glFunctions ) 
                : eq::ObjectManager< const void* >( glFunctions ) {}
        virtual ~ObjectManager(){}
    };

    class VertexBufferState : public mesh::VertexBufferStateOM, 
                              public eqBase::Referenced
    {
    public:
        VertexBufferState( const eq::GLFunctions* glFunctions, 
                           ObjectManager& om )
                : mesh::VertexBufferStateOM( glFunctions, om ) {}
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
        eqBase::RefPtr< ObjectManager >     _objects;
        eqBase::RefPtr< VertexBufferState > _state;

        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
    };
}

#endif // EQ_PLY_WINDOW_H
