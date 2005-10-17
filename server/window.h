
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WINDOW_H
#define EQS_WINDOW_H

#include <iostream>
#include <vector>

namespace eqs
{
    class Channel;
    class Pipe;

    /**
     * The window.
     */
    class Window
    {
    public:
        /** 
         * Constructs a new Window.
         */
        Window();

        /** 
         * Adds a new channel to this window.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel );

        /** 
         * Removes a channel from this window.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed, <code>false</code>
         *         otherwise.
         */
        bool removeChannel( Channel* channel );

        /** 
         * Returns the number of channels on this window.
         * 
         * @return the number of channels on this window. 
         */
        uint nChannels() const { return _channels.size(); }

        /** 
         * Gets a channel.
         * 
         * @param index the channel's index. 
         * @return the channel.
         */
        Channel* getChannel( const uint index ) const
            { return _channels[index]; }

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
        std::vector<Channel*> _channels;

        /** Number of entitities actively using this window. */
        uint _used;

        /** The parent pipe. */
        Pipe* _pipe;
        friend class Pipe;
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
};
#endif // EQS_WINDOW_H
