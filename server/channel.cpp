
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

std::ostream& eqs::operator << ( std::ostream& os, const Channel* channel)
{
    if( !channel )
    {
        os << "NULL channel";
            return os;
    }
    
    os << "channel " << (void*)channel
       << ( channel->isUsed() ? " used" : " unused" );
    return os;
}
