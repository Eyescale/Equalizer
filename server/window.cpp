
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "channel.h"
#include "pipe.h"

using namespace eqs;


Window::Window()
        : _used(0),
          _pipe(NULL)
{
}

Window* Window::clone() const
{
    Window* clone = new Window();

    const uint nChannels = this->nChannels();
    for( uint i=0; i<nChannels; i++ )
    {
        Channel* channel      = getChannel(i);
        Channel* channelClone = channel->clone();

        channelClone->_window = clone;
        clone->_channels.push_back( channelClone );
    }
    return clone;
}

void Window::refUsed()
{
    _used++;
    if( _pipe ) 
        _pipe->refUsed(); 
}
void Window::unrefUsed()
{
    _used--;
    if( _pipe ) 
        _pipe->unrefUsed(); 
}
