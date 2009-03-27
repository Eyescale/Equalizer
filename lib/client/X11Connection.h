
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_X11_CONNECTION_H
#define EQ_X11_CONNECTION_H

#include <eq/client/windowSystem.h>

#ifdef GLX
#  include <eq/client/glXPipe.h>
#endif

#include <eq/net/connection.h>

namespace eq
{
#ifdef GLX
    /**
     * A X11 Display connection wrapper.
     */
    class X11Connection : public net::Connection
    {
    public:
        X11Connection( GLXPipe* pipe_ )
                : pipe( pipe_ )
            {
                EQASSERT( pipe_ );
                _display = pipe_->getXDisplay();

                EQASSERT( _display );
                _state = STATE_CONNECTED;
                EQINFO << "New X11 Connection @" << (void*)this << std::endl;
            }
        
        virtual ~X11Connection() 
            { EQINFO << "Delete X11 connection @" << (void*)this << std::endl; }

        Display* getDisplay() const   { return _display; }
        virtual ReadNotifier getReadNotifier() const
            { return ConnectionNumber( _display ); }

        GLXPipe* const pipe;

    protected:
        virtual int64_t read( void* buffer, const uint64_t bytes )
            { return -1; }
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const
            { return -1; }

    private:
        Display* _display;
    };
#else
    class X11Connection : public eq::net::Connection {};
#endif
}

#endif // EQ_X11_CONNECTION_H
