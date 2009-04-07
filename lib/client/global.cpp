
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "global.h"
#include "nodeFactory.h"

namespace eq
{
EQ_EXPORT NodeFactory* Global::_nodeFactory = 0;
std::string Global::_server;
std::string Global::_configFile;

#ifdef AGL
static base::Lock _carbonLock;
#endif

void Global::enterCarbon()
{
#ifdef AGL
    _carbonLock.set();
#endif
}

void Global::leaveCarbon()
{ 
#ifdef AGL
    _carbonLock.unset();
#endif
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const IAttrValue value )
{
    if( value > ON ) // ugh
        os << static_cast<int>( value );
    else
        os << ( value == UNDEFINED  ? "UNDEFINED" :
                value == OFF        ? "OFF" :
                value == ON         ? "ON" : 
                value == AUTO       ? "AUTO" :
                value == NICEST     ? "NICEST" :
                value == QUAD       ? "QUAD" :
                value == ANAGLYPH   ? "ANAGLYPH" :
                value == VERTICAL   ? "VERTICAL" :
                value == WINDOW     ? "WINDOW" :
                value == PBUFFER    ? "PBUFFER" : 
                value == FBO        ? "FBO" : 
                value == RGBA16F    ? "RGBA16F" :
                value == RGBA32F    ? "RGBA32F" :
                value == ASYNC      ? "ASYNC" : 
                value == DRAW_SYNC  ? "DRAW_SYNC" : 
                value == LOCAL_SYNC ? "LOCAL_SYNC" : 
                "ERROR"  );
    return os;
}
}
