
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_STATEVENT_H
#define EQ_STATEVENT_H

#include <eq/net/object.h>
#include <iostream>

namespace eq
{
    class Channel;

    /**
     * Holds one statistics event, used for profiling.
     */
    class StatEvent 
    {
    public:
        enum Type // Also update string table in statEvent.cpp
        {
            NONE = 0,
            CHANNEL_CLEAR,
            CHANNEL_DRAW,
            CHANNEL_DRAW_FINISH,
            CHANNEL_ASSEMBLE,
            CHANNEL_READBACK,
            CHANNEL_TRANSMIT,
            CHANNEL_TRANSMIT_NODE,
            CHANNEL_WAIT_FRAME,
            TYPE_ALL          // must be last
        };

        StatEvent( const Type type, Channel* channel );
        ~StatEvent();

        struct Data
        {
            Data() : type( NONE ), objectID( EQ_ID_INVALID ) {}
            Data( const Type _type, const uint32_t _objectID )
                    : type( _type ), objectID( _objectID ), 
                      startTime( 0.0f ), endTime( 0.0f ) {}

            Type     type;
            uint32_t objectID;
            float    startTime;
            float    endTime;

        };

        Data data;

    private:
        Channel* _channel;

        static EQ_EXPORT std::string _typeNames[TYPE_ALL];
        friend std::ostream& operator << ( std::ostream&, const Data& );
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const StatEvent::Data& event)
    {
        os << StatEvent::_typeNames[ event.type ] << ":" << event.objectID
           << " " << event.startTime << " - " << event.endTime;
        return os;
    }
}

#endif // EQ_STATEVENT_H
