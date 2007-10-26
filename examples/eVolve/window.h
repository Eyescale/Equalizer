
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_WINDOW_H
#define EVOLVE_WINDOW_H

#include <eq/eq.h>

namespace eVolve
{
    class ObjectManager : public eq::ObjectManager< const void* >, 
                          public eqBase::Referenced
    {
    public:
        ObjectManager( const eq::GLFunctions* glFunctions ) 
                : eq::ObjectManager< const void * >( glFunctions ) {}
        virtual ~ObjectManager(){}
    };

    class Window : public eq::Window
    {
    public:
        Window() : _logoTexture( 0 ) {}

        // display list cache (windows share the context and object manager)
        GLuint getDisplayList( const void* key )
            { return _objects->getList( key ); }
        GLuint newDisplayList( const void* key )
            { return _objects->newList( key ); }

        void getLogoTexture( GLuint& id, vmml::Vector2i& size ) const
            { id = _logoTexture; size = _logoSize; }

    protected:
        virtual ~Window() {}
        virtual bool configInit( const uint32_t initID );
        virtual bool configInitGL( const uint32_t initID );
        virtual bool configExit();

    private:
        eqBase::RefPtr< ObjectManager > _objects;

        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
    };
}

#endif // EVOLVE_WINDOW_H
