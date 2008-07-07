
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"
#include "nodeFactory.h"

using namespace std;

namespace eq
{
EQ_EXPORT NodeFactory* Global::_nodeFactory = 0;
string Global::_server;

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
        os << ( value == UNDEFINED ? "UNDEFINED" :
                value == OFF       ? "OFF" :
                value == ON        ? "ON" : 
                value == AUTO      ? "AUTO" :
                value == NICEST    ? "NICEST" :
                value == QUAD      ? "QUAD" :
                value == ANAGLYPH  ? "ANAGLYPH" :
                value == VERTICAL  ? "VERTICAL" :
                value == WINDOW    ? "WINDOW" :
                value == PBUFFER   ? "PBUFFER" : "ERROR"  );
    return os;
}
}
