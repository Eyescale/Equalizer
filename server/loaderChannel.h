
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_CHANNEL_H
#define EQS_LOADER_CHANNEL_H

#include "loaderState.h"

#include "channel.h"
#include "config.h"

namespace eqLoader
{
    struct newChannelAction
    {
        newChannelAction( State& state ) : _state( state ) {}
        
        void operator()(const char& c) const
            {
                _state.channel = _state.loader->createChannel( );
                _state.window->addChannel( _state.channel );
            }
    
        State& _state;
    };

    struct ChannelGrammar : public grammar<ChannelGrammar>
    {
        ChannelGrammar( State& state ) : _state(state) {}

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> const& start() const 
                { return channel; }

            definition( ChannelGrammar const& self )
                {
                    channel = "channel"
                        >> ch_p('{')//[newChannelAction(self._state)]
                        >> ch_p('}');
                }

            rule<ScannerT> channel;
        };

        State&        _state;
    };

    inline ChannelGrammar channel_p( State& state )
    { 
        return ChannelGrammar( state ); 
    }
}

#endif // EQS_LOADER_CHANNEL_H
