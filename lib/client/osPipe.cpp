
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "osPipe.h"

#include <eq/base/debug.h>

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
