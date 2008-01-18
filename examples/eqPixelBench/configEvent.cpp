
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configEvent.h"

using namespace std;

namespace eqPixelBench
{
std::ostream& operator << ( std::ostream& os, const ConfigEvent* event )
{
    switch( event->data.type )
    {
        case ConfigEvent::READBACK:
            os  << "readback";
            break;

        case ConfigEvent::ASSEMBLE:
            os  << "assemble";
            break;

        case ConfigEvent::START_LATENCY:
            os  << "        ";
            break;

        default:
            os << static_cast< const eq::ConfigEvent* >( event );
            return os;
    }

    os << " \"" << event->data.user.data << "\" " << event->formatType
       << string( 50-strlen( event->formatType ), ' ' ) << event->area << ": ";

    if( event->msec < 0.0f )
        os << "error 0x" << hex << static_cast< int >( -event->msec ) << dec;
    else
        os << static_cast< uint32_t >( event->area.getArea() / event->msec
                                       / 1048.576f )
           << "MPix/sec (" << event->msec << "ms, " << 1000.0f / event->msec
           << "FPS)";
    return os;
}
}
