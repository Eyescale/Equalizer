
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_WINDOW_H
#define EQ_PLY_WINDOW_H

#include <eq/eq.h>

namespace eqPly
{
    class ObjectManager : public eq::ObjectManager< const void* >, 
                          public eqBase::Referenced
    {};

    class Window : public eq::Window
    {
    public:
        // display list cache (windows share the context and object manager)
        GLuint getDisplayList( const void* key )
            { return _objects->getList( key ); }
        GLuint newDisplayList( const void* key )
            { return _objects->newList( key ); }

    protected:
        virtual ~Window() {}
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

    private:
        eqBase::RefPtr< ObjectManager > _objects;
    };
}

#endif // EQ_PLY_WINDOW_H
