
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "windowEvent.h"

using namespace eqBase;
using namespace std;

namespace eq
{
/** String representation of event types. */
static std::string _windowEventTypeNames[WindowEvent::ALL] =
{
    "EXPOSE",
    "RESIZE",
    "POINTER_MOTION",
    "POINTER_BUTTON_PRESS",
    "POINTER_BUTTON_RELEASE",
    "KEY_PRESS",
    "KEY_RELEASE",
    "CLOSE",
    "UNHANDLED",
};

std::ostream& operator << ( std::ostream& os, const WindowEvent& event )
{
    os << disableFlush << _windowEventTypeNames[event.type] << " ";
    switch( event.type )
    {
        case WindowEvent::EXPOSE:
        case WindowEvent::CLOSE:
            break;

        case WindowEvent::RESIZE:
            os << event.resize;
            break;

        case WindowEvent::POINTER_MOTION:
        case WindowEvent::POINTER_BUTTON_PRESS:
        case WindowEvent::POINTER_BUTTON_RELEASE:
            os << event.pointerEvent;
            break;

        case WindowEvent::KEY_PRESS:
        case WindowEvent::KEY_RELEASE:
            os << event.keyEvent;
            break;

        default:
            break;
    }
    
    os << enableFlush << endl;
    return os;
}
}
