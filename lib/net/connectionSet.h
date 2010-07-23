
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

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#ifdef EQ_EXPORTS
   // We need to instantiate a Monitor< Event > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif

#include <eq/net/connectionListener.h> // base class

#include <eq/base/base.h>
#include <eq/base/buffer.h>
#include <eq/base/hash.h>
#include <eq/base/monitor.h>
#include <eq/base/refPtr.h>

#ifdef WIN32
#  include <eq/base/thread.h>
#else
#  include <poll.h>
#endif

namespace eq
{
namespace net
{
    class Message;
    class Network;
    class EventConnection;

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

        EQ_EXPORT ConnectionSet();
        EQ_EXPORT ~ConnectionSet();

        EQ_EXPORT void addConnection( ConnectionPtr connection );
        EQ_EXPORT bool removeConnection( ConnectionPtr connection );
        EQ_EXPORT void clear();
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
        EQ_EXPORT Event select( const int timeout = -1 );

        /**
         * Interrupt the current or next select call.
         */
        EQ_EXPORT void interrupt();

        /** @internal Trigger rebuilding of internal caches. */
        void setDirty();

        int           getError()     { return _error; }
        ConnectionPtr getConnection(){ return _connection; }

    private:
 
#ifdef WIN32
        /** Handles connections exceeding MAXIMUM_WAIT_OBJECTS */
        class Thread : public eq::base::Thread
        {
        public:
            Thread( ConnectionSet* parent );
            virtual ~Thread();

            ConnectionSet* set;
            HANDLE         notifier;
            
            base::Monitor< Event > event;

        protected:
            virtual void run();

        private:
            ConnectionSet* const _parent;
        };


        typedef std::vector< Thread* > Threads;
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
        base::Lock _mutex;

        /** The connections of this set */
        Connections _allConnections;

        /** The connections to handle */
        Connections _connections;

        // Note: std::vector had to much overhead here
#ifdef WIN32
        base::Buffer< HANDLE > _fdSet;
#else
        base::Buffer< pollfd > _fdSetCopy; // 'const' set
        base::Buffer< pollfd > _fdSet;     // copy of _fdSetCopy used to poll
#endif
        base::Buffer< Result > _fdSetResult;

        /** The connection to reset a running select, see constructor. */
        base::RefPtr< EventConnection > _selfConnection;

        // result values
        ConnectionPtr _connection;
        int           _error;

        /** FD sets need rebuild. */
        bool _dirty;

        bool _setupFDSet();
        bool _buildFDSet();
        virtual void notifyStateChanged( Connection* ) { _dirty = true; }

        Event _getSelectResult( const uint32_t index );
        CHECK_THREAD_DECLARE( _selectThread );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet* set );
    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet::Event event );
}
}

#endif // EQNET_CONNECTION_SET_H
