
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

void Window::addChannel( Channel* channel )
{
    _channels.push_back( channel ); 
    channel->_window = this; 
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

std::ostream& eqs::operator << ( std::ostream& os, const Window* window )
{
    if( !window )
    {
        os << "NULL window";
        return os;
    }
    
    const uint nChannels = window->nChannels();
    os << "window " << (void*)window
       << ( window->isUsed() ? " used " : " unused " ) << nChannels
       << " channels";
    
    for( uint i=0; i<nChannels; i++ )
        os << std::endl << "    " << window->getChannel(i);
    
    return os;
}
