
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "osPipe.h"

namespace eq
{

OSPipe::OSPipe( Pipe* parent )
    : _pipe( parent )
#ifdef WGL
    , _wglewContext( new WGLEWContext )
#else
    , _wglewContext( 0 )
#endif
{
    EQASSERT( _pipe ); 
}

OSPipe::~OSPipe()
{
    delete _wglewContext;
    _wglewContext = 0;
}

}
