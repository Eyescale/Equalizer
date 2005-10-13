
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

//#include "channel.h"
#include "node.h"
#include "window.h"

using namespace eqs;


Pipe::Pipe()
        : _used( 0 ),
          _node( NULL )
{}

Pipe* Pipe::clone() const
{
    Pipe* clone = new Pipe();

    const uint nWindows = this->nWindows();
    for( uint i=0; i<nWindows; i++ )
    {
        Window* window      = getWindow(i);
        Window* windowClone = window->clone();

        windowClone->_pipe = clone;
        clone->_windows.push_back( windowClone );
    }
    return clone;
}

void Pipe::refUsed()
{
    _used++;
    if( _node ) 
        _node->refUsed(); 
}
void Pipe::unrefUsed()
{
    _used--;
    if( _node ) 
        _node->unrefUsed(); 
}
