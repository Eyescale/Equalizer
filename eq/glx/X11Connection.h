
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/os.h>
#include <co/connection.h>

namespace eq
{
namespace glx
{
    /**
     * @internal
     * An X11 Display connection wrapper.
     *
     * This class is used to monitor multiple X11 display connections for events
     * using a co::ConnectionSet.
     */
    class X11Connection : public co::Connection
    {
    public:
        X11Connection( Display* display )
                : _display( display ) { _setState( STATE_CONNECTED ); }
        
        virtual ~X11Connection() { _setState( STATE_CLOSED ); }

        virtual Notifier getNotifier() const
            { return ConnectionNumber( _display ); }

        const Display* getDisplay() const { return _display; }

    protected:
        virtual void readNB( void*, const uint64_t ) { LBDONTCALL; }
        virtual int64_t readSync( void*, const uint64_t, const bool )
            { LBDONTCALL; return -1; }
        virtual int64_t write( const void*, const uint64_t )
            { LBDONTCALL; return -1; }

    private:
        Display* const _display;
    };
}
}

#endif // EQ_X11_CONNECTION_H
