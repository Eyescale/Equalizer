
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include <eq/net/connectionListener.h> // base class

#include <eq/base/base.h>
#include <eq/base/hash.h>
#include <eq/base/refPtr.h>

#ifndef WIN32
#  include <poll.h>
#endif

namespace eqNet
{
    class Message;
    class Network;
    class PipeConnection;

    /**
     * A set of connections. 
     *
     * From the set, a connection with pending events can be selected.
     */
    class EQ_EXPORT ConnectionSet : public ConnectionListener
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

        ConnectionSet();
        ~ConnectionSet();

        void addConnection( ConnectionPtr connection );
        bool removeConnection( ConnectionPtr connection );
        void clear();
        size_t size()  const { return _connections.size(); }
        bool   empty() const { return _connections.empty(); }

        const ConnectionVector& getConnections() const { return _connections; }

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
        Event select( const int timeout = -1 );

        /**
         * Interrupt the current or next select call.
         */
        void interrupt();

        int           getError()     { return _error; }
        ConnectionPtr getConnection(){ return _connection; }

    private:
        /** Mutex protecting changes to the set. */
        eqBase::SpinLock _mutex;

        /** The connections to handle */
        ConnectionVector _connections;

#ifdef WIN32
        std::vector<HANDLE> _fdSet;
#else
        std::vector<pollfd> _fdSetCopy; // 'const' set
        std::vector<pollfd> _fdSet;     // copy of _fdSetCopy used to poll()
#endif
        stde::hash_map<Connection::ReadNotifier, ConnectionPtr>
            _fdSetConnections;

        enum SelfCommands
        {
            SELF_INTERRUPT = 42
        };

        /** The connection to reset a running select, see constructor. */
        eqBase::RefPtr< PipeConnection > _selfConnection;

        // result values
        ConnectionPtr _connection;
        int           _error;

        /** FD sets need rebuild. */
        bool _dirty;

        bool _getEvent( Event& event, Connection::ReadNotifier& fd );
        void _dirtyFDSet();
        bool _setupFDSet();
        bool _buildFDSet();
        virtual void notifyStateChanged( Connection* ) { _dirty = true; }

        Event _handleSelfCommand();
        Event _getSelectResult( const uint32_t index );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet* set );
    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet::Event event );
}

#endif // EQNET_CONNECTION_SET_H
