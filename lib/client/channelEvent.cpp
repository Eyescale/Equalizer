
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channelEvent.h"

using namespace eq::base;
using namespace std;

namespace eq
{

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const ChannelEvent& event )
{
    os << event.data << endl;
    return os;
}
}
