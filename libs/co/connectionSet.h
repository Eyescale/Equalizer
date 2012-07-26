
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef CO_CONNECTION_SET_H
#define CO_CONNECTION_SET_H

#include <co/api.h>
#include <co/types.h>
#include <lunchbox/thread.h> // for LB_TS_VAR

namespace co
{
namespace detail { class ConnectionSet; }

    /** Handles events on a set of connections. */
    class ConnectionSet
    {
    public:
        enum Event //!< Event types for select()
        {
            EVENT_NONE = 0,        //!< No event has occurred
            EVENT_CONNECT,         //!< A new connection
            EVENT_DISCONNECT,      //!< A disconnect
            EVENT_DATA,            //!< Data can be read
            EVENT_TIMEOUT,         //!< The selection request timed out
            EVENT_INTERRUPT,       //!< ConnectionSet::interrupt was called
            EVENT_ERROR,           //!< A connection signaled an error
            EVENT_SELECT_ERROR,    //!< An error occurred during select()
            EVENT_INVALID_HANDLE,  //!< A connection is not select'able
            EVENT_ALL
        };

        /** Create a new connection set. @version 1.0 */
        CO_API ConnectionSet();

        /** Destruct this connection set. @version 1.0 */
        CO_API ~ConnectionSet();

        /** @name Managing connections. */
        //@{
        /** Add the connection to this set. Thread-safe. @version 1.0 */
        CO_API void addConnection( ConnectionPtr connection );

        /** Remove the connection from this set. Thread-safe. @version 1.0 */
        CO_API bool removeConnection( ConnectionPtr connection );

        /** Remove all connections from this set. Thread-safe. @version 1.0 */
        CO_API size_t getSize() const;

        /**
         * @return true if this set has no connections. Thread-safe.
         * @version 1.0
         */
        CO_API bool isEmpty() const;

        /** @internal not thread-safe. */
        CO_API const Connections& getConnections() const;
        //@}

        /** @name Performing a selection. */
        //@{
        /**
         * Select a Connection which is ready for I/O.
         *
         * Depending on the event, the error number and connection are set.
         *
         * @param timeout the timeout to wait for an event in milliseconds,
         *                or LB_TIMEOUT_INDEFINITE if the call should block
         *                forever.
         * @return The type of the event occured during selection.
         * @sa getConnection(), getError()
         * @version 1.0
         */
        CO_API Event select( const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

        /** Interrupt the current or next select call. @version 1.0 */
        CO_API void interrupt();

        /**
         * @return the error code when the last select() returned EVENT_ERROR.
         * @version 1.0
         */
        CO_API int getError() const;

        /**
         * @return the connection of the last select event, may be 0.
         * @version 1.0
         */
        CO_API ConnectionPtr getConnection();

        /** @internal Trigger rebuilding of internal caches. */
        void setDirty();
        //@}

    private:
        detail::ConnectionSet* const _impl;

        void _clear();
        bool _setupFDSet();
        bool _buildFDSet();

        Event _getSelectResult( const uint32_t index );
        LB_TS_VAR( _selectThread );
    };

    /** @internal */
    CO_API std::ostream& operator << ( std::ostream&, const ConnectionSet& );

    /** @internal */
    CO_API std::ostream& operator << (std::ostream&,const ConnectionSet::Event);
}

#endif // CO_CONNECTION_SET_H
