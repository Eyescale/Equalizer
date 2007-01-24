
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_STATEVENT_H
#define EQ_STATEVENT_H

#include <eq/net/object.h>
#include <iostream>

namespace eq
{
    /**
     * Holds one statistics event, used for profiling.
     */
    class StatEvent 
    {
    public:
        enum Type // Also update string table in statEvent.cpp
        {
            CHANNEL_CLEAR,
            CHANNEL_DRAW,
            CHANNEL_ASSEMBLE,
            CHANNEL_READBACK,
            CHANNEL_TRANSMIT,
            CHANNEL_WAIT_FRAME,
            TYPE_ALL          // must be last
        };

        StatEvent(){}
        StatEvent( const Type _type, const eqNet::Object* object, 
                   const float _startTime )
                : type( _type ), objectType( object->getTypeID( )),
                  objectID( object->getID( )), startTime( _startTime ) {}

        Type     type;
        uint32_t objectType;
        uint32_t objectID;
        float    startTime;
        float    endTime;

        static EQ_EXPORT std::string typeNames[TYPE_ALL];
    };

    inline std::ostream& operator << ( std::ostream& os, const StatEvent& event)
    {
        os << " event " << StatEvent::typeNames[ event.type ] << " object "
           << event.objectType << ":" << event.objectID << " " 
           << event.startTime << " - " << event.endTime;
        return os;
    }
}

#endif // EQ_STATEVENT_H
