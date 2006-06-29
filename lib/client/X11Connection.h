
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_X11_CONNECTION_H
#define EQ_X11_CONNECTION_H

#include <eq/base/userdata.h>
#include <eq/net/connection.h>

namespace eq
{
#ifdef GLX
    /**
     * A X11 Display connection wrapper.
     */
    class X11Connection : public eqNet::Connection, 
                          public virtual eqBase::Userdata<Pipe*>
    {
    public:
        X11Connection( Display* display )
                : _display( display )
            {
                _state = STATE_CONNECTED;
            }

        Display* getDisplay() const   { return _display; }
        virtual int getReadFD() const { return ConnectionNumber( _display ); }

    private:
        Display* _display;
    };
#else
    class X11Connection : public eqNet::Connection {};
#endif
}

#endif // EQ_X11_CONNECTION_H
