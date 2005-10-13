
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "window.h"

#include <eq/base/base.h>

using namespace eqs;
using namespace eqBase;


Channel::Channel()
        : _used(0),
          _window(NULL)
{
}

Channel* Channel::clone()
{
    return new Channel();
}

void Channel::refUsed()
{
    _used++;
    if( _window ) 
        _window->refUsed(); 
}

void Channel::unrefUsed()
{
    ASSERT( _used != 0 );
    _used--;
    if( _window ) 
        _window->unrefUsed(); 
}
