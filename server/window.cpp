
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "channel.h"

using namespace eqs;


Window::Window(const Window& from)
{
    const uint nChannels = from.nChannels();
    for( uint i=0; i<nChannels; i++ )
    {
        Channel* channel = from.getChannel(i);
        _channels.push_back( new Channel( *channel ));
    }
}
