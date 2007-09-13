
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_WINDOW_H
#define EQ_VOL_WINDOW_H

#include <eq/eq.h>

namespace eqVol
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

        bool configInitGL( const uint32_t initID );

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
        virtual bool configExit();

    private:
        eqBase::RefPtr< ObjectManager > _objects;

        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
    };
}

#endif // EQ_VOL_WINDOW_H
