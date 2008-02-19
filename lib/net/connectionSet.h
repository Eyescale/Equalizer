
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_SET_H
#define EQNET_CONNECTION_SET_H

#include <eq/net/connection.h>

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
        
    /**
     * A set of connections. 
     *
     * From the set, a connection with pending events can be selected.
     */
    class EQ_EXPORT ConnectionSet
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

        void addConnection( eqBase::RefPtr<Connection> connection );
        bool removeConnection( eqBase::RefPtr<Connection> connection );
        void clear();
        size_t size() const { return _connections.size(); }

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

        int                        getError()     { return _error; }
        eqBase::RefPtr<Connection> getConnection(){ return _connection; }

    private:
        /** Mutex protecting changes to the set. */
        eqBase::SpinLock _mutex;

        /** The connections to handle */
        ConnectionVector _connections;

#ifdef WIN32
        std::vector<HANDLE> _fdSet;
#else
        std::vector<pollfd> _fdSet;
#endif
        stde::hash_map<Connection::ReadNotifier, Connection*> _fdSetConnections;

        enum SelfCommands
        {
            SELF_MODIFIED = 1,
            SELF_INTERRUPT
        };

        /** The connection to reset a running select, see constructor. */
        eqBase::RefPtr<Connection> _selfConnection;

        // result values
        eqBase::RefPtr<Connection> _connection;
        int                        _error;

        bool _getEvent( Event& event, Connection::ReadNotifier& fd );
        void _dirtyFDSet();
        bool _setupFDSet();
        bool _buildFDSet();

        Event _handleSelfCommand();
        Event _getSelectResult( const uint32_t index );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet* set );
    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const ConnectionSet::Event event );
}

#endif // EQNET_CONNECTION_SET_H
