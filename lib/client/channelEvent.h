
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNELEVENT_H
#define EQ_CHANNELEVENT_H

#include <eq/client/event.h>

namespace eq
{
    class Channel;

    /** A channel-system event for an eq::Channel */
    class EQ_EXPORT ChannelEvent
    {
    public:

        Channel* channel;
        Event    data;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ChannelEvent& event );
}

#endif // EQ_CHANNELEVENT_H

