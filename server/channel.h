
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include <iostream>
#include <vector>

namespace eqs
{
    /**
     * The channel.
     */
    class Channel
    {
    public:
        /** 
         * Constructs a new Channel.
         */
        Channel(); 

        /** 
         * Constructs a new deep copy of another channel.
         * 
         * @param from the original channel.
         */
        Channel(const Channel& from);
    };

    inline std::ostream& operator << ( std::ostream& os, const Channel* channel)
    {
        if( !channel )
        {
            os << "NULL channel";
            return os;
        }

        os << "channel " << (void*)channel;
        return os;
    }
};
#endif // EQS_CHANNEL_H
