
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "pipe.h"

namespace eqPly
{

bool Window::configInit( const uint32_t initID )
{
    if( !eq::Window::configInit( initID ))
        return false;

    eq::Pipe*  pipe        = getPipe();
    Window*    firstWindow = static_cast< Window* >( pipe->getWindow( 0 ));

    _objects = firstWindow->_objects;
    if( !_objects )
        _objects = new ObjectManager;

    return true;
}
bool Window::configExit()
{
    if( _objects.isValid() && _objects->getRefCount() == 1 )
        _objects->deleteAll();

    _objects = 0;
    return eq::Window::configExit();
}
}
