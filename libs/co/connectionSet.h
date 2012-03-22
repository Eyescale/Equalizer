
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

#include <co/connectionListener.h> // base class
#include <lunchbox/thread.h> // for EQ_TS_VAR

namespace co
{
namespace detail { class ConnectionSet; }

    /** Handles events on a set of connections. */
    class ConnectionSet : public ConnectionListener
    {
    public:
        enum Event
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

        CO_API ConnectionSet();
        CO_API ~ConnectionSet();

        CO_API void addConnection( ConnectionPtr connection );
        CO_API bool removeConnection( ConnectionPtr connection );
        CO_API void clear();
        CO_API size_t getSize() const;
        CO_API bool isEmpty() const;

        CO_API const Connections& getConnections() const;

        /** 
         * Selects a Connection which is ready for I/O.
         *
         * Depending on the event, the error number and connection are set.
         * 
         * @param timeout the timeout to wait for an event in milliseconds,
         *                or EQ_TIMEOUT_INDEFINITE if the call should block
         *                forever.
         * @return The event occured during selection.
         */
        CO_API Event select( const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /**
         * Interrupt the current or next select call.
         */
        CO_API void interrupt();

        /** @internal Trigger rebuilding of internal caches. */
        void setDirty();

        CO_API int getError() const;
        CO_API ConnectionPtr getConnection();

    private:
        detail::ConnectionSet* const _impl;

        bool _setupFDSet();
        bool _buildFDSet();
        virtual void notifyStateChanged( Connection* );

        Event _getSelectResult( const uint32_t index );
        EQ_TS_VAR( _selectThread );
    };

    CO_API std::ostream& operator << ( std::ostream& os, 
                                       const ConnectionSet* set );
    CO_API std::ostream& operator << ( std::ostream& os, 
                                       const ConnectionSet::Event event );
}

#endif // CO_CONNECTION_SET_H
