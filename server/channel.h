
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_H
#define EQS_CHANNEL_H

#include <iostream>
#include <vector>

namespace eqs
{
    class Window;

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
         * References this window as being actively used.
         */
        void refUsed();

        /** 
         * Unreferences this window as being actively used.
         */
        void unrefUsed();

        /** 
         * Returns if this window is actively used.
         *
         * @return <code>true</code> if this window is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

    private:
        /** Number of entitities actively using this channel. */
        uint _used;

        /** The parent window. */
        Window* _window;
        friend class Window;
    };

    std::ostream& operator << ( std::ostream& os, const Channel* channel);
};
#endif // EQS_CHANNEL_H
