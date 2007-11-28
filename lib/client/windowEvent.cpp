
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "windowEvent.h"

using namespace eqBase;
using namespace std;

namespace eq
{

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const WindowEvent& event )
{
    os << event.data << endl;
    return os;
}
}
