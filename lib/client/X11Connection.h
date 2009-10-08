
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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
     * An X11 Display connection wrapper.
     *
     * This class is used to monitor multiple GLXPipe X11 display connections
     * for events using a net::ConnectionSet.
     * @internal
     */
    class X11Connection : public net::Connection
    {
    public:
        X11Connection( GLXPipe* pipe_ )
                : pipe( pipe_ )
            {
                EQASSERT( pipe_ );
                EQASSERT( pipe_->getXDisplay( ));

                _state = STATE_CONNECTED;
                EQINFO << "New X11 Connection @" << (void*)this << std::endl;
            }
        
        virtual ~X11Connection() 
            { EQINFO << "Delete X11 connection @" << (void*)this << std::endl; }

        virtual Notifier getNotifier() const
            {
                Display* const display = pipe->getXDisplay();
                return display ? ConnectionNumber( display ) : -1;
            }

        GLXPipe* const pipe;

    protected:
        virtual void readNB( void* buffer, const uint64_t bytes )
            { EQDONTCALL; }
        virtual int64_t readSync( void* buffer, const uint64_t bytes )
            { EQDONTCALL; return -1; }
        virtual int64_t write( const void* buffer, const uint64_t bytes )
            { EQDONTCALL; return -1; }

    private:
    };
#else
    class X11Connection : public eq::net::Connection {};
#endif
}

#endif // EQ_X11_CONNECTION_H
