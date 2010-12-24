
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <co/base/os.h>
#include <co/base/buffer.h>
#include <co/base/hash.h>
#include <co/base/monitor.h>
#include <co/base/refPtr.h>
#include <co/base/thread.h> // for EQ_TS_VAR

#ifndef _WIN32
#  include <poll.h>
#endif

namespace co
{
    class EventConnection;
    class ConnectionSetThread;

    /**
     * A set of connections. 
     *
     * From the set, a connection with pending events can be selected.
     */
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
        size_t getSize()  const { return _connections.size(); }
        bool   isEmpty() const { return _connections.empty(); }

        const Connections& getConnections() const{ return _allConnections; }

        /** 
         * Selects a Connection which is ready for I/O.
         *
         * Depending on the event, the error number and connection are set.
         * 
         * @param timeout the timeout to wait for an event in milliseconds,
         *                or <code>-1</code> if the call should block
         *                indefinitly.
         * @return The event occured during selection.
         */
        CO_API Event select( const int timeout = -1 );

        /**
         * Interrupt the current or next select call.
         */
        CO_API void interrupt();

        /** @internal Trigger rebuilding of internal caches. */
        void setDirty();

        int           getError()     { return _error; }
        ConnectionPtr getConnection(){ return _connection; }

    private:
 
#ifdef _WIN32
        typedef ConnectionSetThread Thread;
        typedef std::vector< ConnectionSetThread* > Threads;
        /** Threads used to handle more than MAXIMUM_WAIT_OBJECTS connections */
        Threads _threads;

        /** Result thread. */
        Thread* _thread;

        union Result
        {
            Connection* connection;
            Thread* thread;
        };

#else
        union Result
        {
            Connection* connection;
        };
#endif

        /** Mutex protecting changes to the set. */
        co::base::Lock _mutex;

        /** The connections of this set */
        Connections _allConnections;

        /** The connections to handle */
        Connections _connections;

        // Note: std::vector had to much overhead here
#ifdef _WIN32
        co::base::Buffer< HANDLE > _fdSet;
#else
        co::base::Buffer< pollfd > _fdSetCopy; // 'const' set
        co::base::Buffer< pollfd > _fdSet;     // copy of _fdSetCopy used to poll
#endif
        co::base::Buffer< Result > _fdSetResult;

        /** The connection to reset a running select, see constructor. */
        co::base::RefPtr< EventConnection > _selfConnection;

        // result values
        ConnectionPtr _connection;
        int           _error;

        /** FD sets need rebuild. */
        bool _dirty;

        bool _setupFDSet();
        bool _buildFDSet();
        virtual void notifyStateChanged( Connection* ) { _dirty = true; }

        Event _getSelectResult( const uint32_t index );
        EQ_TS_VAR( _selectThread );
    };

    CO_API std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet* set );
    CO_API std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet::Event event );
}

#endif // CO_CONNECTION_SET_H
