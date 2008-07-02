
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_X11_CONNECTION_H
#define EQ_X11_CONNECTION_H

#include <eq/client/windowSystem.h>

#include <eq/net/connection.h>

namespace eq
{
#ifdef GLX
    /**
     * A X11 Display connection wrapper.
     */
    class X11Connection : public eqNet::Connection
    {
    public:
        X11Connection( Pipe* pipe_ )
                : pipe( pipe_ )
                , _display( pipe_->getXDisplay( ))
            {
                EQASSERT( _display );
                _state = STATE_CONNECTED;
                EQINFO << "New X11 Connection @" << (void*)this << std::endl;
            }
        
        virtual ~X11Connection() 
            { EQINFO << "Delete X11 connection @" << (void*)this << std::endl; }

        Display* getDisplay() const   { return _display; }
        virtual ReadNotifier getReadNotifier() const
            { return ConnectionNumber( _display ); }

        Pipe* const pipe;

    protected:
        virtual int64_t read( void* buffer, const uint64_t bytes )
            { return -1; }
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const
            { return -1; }

    private:
        Display* _display;
    };
#else
    class X11Connection : public eqNet::Connection {};
#endif
}

#endif // EQ_X11_CONNECTION_H
