
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WINDOW_H
#define EQS_WINDOW_H

#include <iostream>
#include <vector>

namespace eqs
{
    class Channel;

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
         * Constructs a new deep copy of another window.
         * 
         * @param from the original window.
         */
        Window(const Window& from);

        /** 
         * Adds a new channel to this window.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel ){ _channels.push_back( channel ); }

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

    private:
        std::vector<Channel*> _channels;
    };

    inline std::ostream& operator << ( std::ostream& os, const Window* window )
    {
        if( !window )
        {
            os << "NULL window";
            return os;
        }

        const uint nChannels = window->nChannels();
        os << "window " << (void*)window << " " << nChannels << " channels";
        for( uint i=0; i<nChannels; i++ )
            os << std::endl << "    " << window->getChannel(i);

        return os;
    }
};
#endif // EQS_WINDOW_H
