
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_CONFIGEVENT_H
#define EQ_PIXELBENCH_CONFIGEVENT_H

#include <eq/eq.h>

struct ConfigEvent : public eq::ConfigEvent
{
public:
    enum Type
    {
        READBACK = eq::ConfigEvent::USER,
        ASSEMBLE
    };

    ConfigEvent()
        {
            size = sizeof( ConfigEvent );
        }

    // channel name is in user event data
    char           formatType[64];
    vmml::Vector2i area;
    float          msec;
};

std::ostream& operator << ( std::ostream& os, const ConfigEvent* event );

#endif // EQ_PIXELBENCH_CONFIGEVENT_H

