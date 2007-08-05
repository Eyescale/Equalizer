
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "event.h"

using namespace std;

namespace eq
{
std::ostream& operator << ( std::ostream& os, const ResizeEvent& event )
{
    os << event.x << "x" << event.y << "+" << event.w << "+" << event.h << " ";
    return os;
}

std::ostream& operator << ( std::ostream& os, const PointerEvent& event )
{
    os << event.x << "x" << event.y << " d(" << event.dx << "x" << event.dy 
       << ")" << " buttons ";

    if( event.buttons == PTR_BUTTON_NONE ) os << "none";
    if( event.buttons & PTR_BUTTON1 ) os << "1";
    if( event.buttons & PTR_BUTTON2 ) os << "2";
    if( event.buttons & PTR_BUTTON3 ) os << "3";
    if( event.buttons & PTR_BUTTON4 ) os << "4";
    if( event.buttons & PTR_BUTTON5 ) os << "5";

    os << " fired ";
    if( event.button == PTR_BUTTON_NONE ) os << "none";
    if( event.button & PTR_BUTTON1 ) os << "1";
    if( event.button & PTR_BUTTON2 ) os << "2";
    if( event.button & PTR_BUTTON3 ) os << "3";
    if( event.button & PTR_BUTTON4 ) os << "4";
    if( event.button & PTR_BUTTON5 ) os << "5";

    os << " ";
    return os;
}

std::ostream& operator << ( std::ostream& os, const KeyEvent& event )
{
    os << "key " << event.key << " " << endl;
    return os;
}
}
