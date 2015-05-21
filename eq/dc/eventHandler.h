
/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef EQ_DC_EVENTHANDLER_H
#define EQ_DC_EVENTHANDLER_H

#include <eq/client/eventHandler.h> // base class
#include <eq/client/types.h>        // basic typedefs

#include <lunchbox/thread.h> // thread-safety macro

namespace eq
{
namespace dc
{
    /** @internal The event handler for one DisplayCluster view. */
    class EventHandler : public eq::EventHandler
    {
    public:
        /** Construct a new DisplayCluster event handler. @version 1.7.1 */
        EventHandler( Proxy* proxy );

        /** Destruct the DisplayCluster event handler. @version 1.7.1 */
        virtual ~EventHandler();

        /**
         * Dispatch all pending events on the current thread.
         *
         * If no event handlers have been constructed by the calling thread,
         * this function does nothing. This function does not block on events.
         *
         * @param proxy if not 0, limit processing to the given dcProxy instance
         * @version 1.7.1
         */
        static void processEvents( const Proxy* proxy = 0 );

    private:
        /** The corresponding DisplayCluster proxy instance. */
        Proxy* const _proxy;

        void _processEvents( const Proxy* proxy );

        LB_TS_VAR( _thread );
    };
}
}
#endif // EQ_DC_EVENTHANDLER_H

