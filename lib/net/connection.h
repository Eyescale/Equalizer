
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include <eq/net/connectionDescription.h>
#include <eq/net/packets.h>

#include <eq/base/base.h>
#include <eq/base/lock.h>
#include <eq/base/referenced.h>
#include <eq/base/refPtr.h>
#include <eq/base/scopedMutex.h>

#include <stdexcept>
#include <sys/types.h>
#include <vector>

#ifdef WIN32
#  include <malloc.h>     // for alloca()
#else
#  include <sys/param.h>  // for MAX()
#endif

#ifdef WIN32
#  define EQ_DEFAULT_PORT (4242)
#else
#  define EQ_DEFAULT_PORT (4242 + getuid())
#endif

namespace eqNet
{
    enum ConnectionType;

    /**
     * A base class to provide communication to other hosts.
     */
    class EQ_EXPORT Connection : public eqBase::Referenced
    {
    public:
        enum State {
            STATE_CLOSED,
            STATE_CONNECTING,
            STATE_CONNECTED,
            STATE_LISTENING
        };

        /** 
         * Creates a new connection.
         *
         * This factory method creates a new concrete connection for the
         * requested type. The concrete connection may not support all
         * functionality of the Connection interface.
         * 
         * @param type the connection type.
         * @return the connection.
         */
        static eqBase::RefPtr<Connection> create( const ConnectionType type );
        
        /** @name Connection Management */
        //@{
        /** 
         * Connect the connection.
         *
         * @return <code>true</code> if the connection was successfully
         *         connected, <code>false</code> if not.
         */
        virtual bool connect()
            { return false; }
        
        /** 
         * Put the connection into the listening state for a new incoming
         * connection.
         *
         * @return <code>true</code> if the connection is listening for new
         *         incoming connections, <code>false</code> if not.
         */
        virtual bool listen()
            { return false; }

        /** 
         * Accepts the next incoming connection.
         * 
         * @return the accepted connection, or <code>NULL</code> if no
         *         connection was accepted.
         */
        virtual eqBase::RefPtr<Connection> accept(){ return NULL; }

        /** 
         * Accepts the next incoming connection with a timeout.
         * 
         * @param timeout the amount of time to wait in milliseconds, if set
         *                to <code>-1</code> the method blocks indefinitely.
         * @return the accepted connection, or <code>NULL</code> if no
         *         connection was accepted.
         */
        virtual eqBase::RefPtr<Connection> accept( const int timeout );

        /** 
         * Closes a connected or listening connection.
         */
        virtual void close(){};
        //@}


        /** @name Messaging API */
        //@{
        /** 
         * Read data from the connection.
         * 
         * @param buffer the buffer for saving the message.
         * @param bytes the number of bytes to read.
         * @return true if all data has been read, false if not.
         */
        bool recv( void* buffer, const uint64_t bytes );

        /** Lock the connection, no other thread can send data. */
        void lockSend()   { _sendLock.set(); }
        /** Unlock the connection. */
        void unlockSend() { _sendLock.unset(); }
            
        /** 
         * Sends data using the connection.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to send.
         * @param isLocked true if the connection is locked externally.
         * @return true if all data has been read, false if not.
         */
        bool send( const void* buffer, const uint64_t bytes, 
                       bool isLocked = false ) const;

        /** 
         * Sends a packaged message using the connection.
         * 
         * @param packet the message packet.
         * @return true if all data has been read, false if not.
         */
        bool send( const Packet &packet ) const
            { return send( &packet, packet.size); }

        /** 
         * Sends a packaged message including a string using the connection.
         * 
         * @param packet the message packet.
         * @param string the string.
         * @return true if all data has been read, false if not.
         */
        bool send( Packet &packet, const std::string& string ) const
            { return send( packet, string.c_str(), string.size()+1 ); }

        /** 
         * Sends a packaged message including additional data.
         *
         * The last item of the packet has to be able to hold one item or eight
         * bytes of the data, whatever is bigger.
         * 
         * @param packet the message packet.
         * @param data the vector containing the data.
         * @return true if all data has been read, false if not.
         */
        template< typename T >
        bool send( Packet &packet, const std::vector<T>& data ) const;

        /** 
         * Sends a packaged message including additional data using the
         * connection.
         * 
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @return true if all data has been read, false if not.
         */
        bool send( Packet& packet, const void* data, const uint64_t size )
            const;

        /** 
         * Sends a packaged message to multiple receivers.
         *
         * The receivers have to implement getConnection(). The reason we don't
         * use an abstract class to define the interface is that we sometimes
         * use a vector of RefPtr<SomeThing> in some places.
         * 
         * @param receivers The receiving entities.
         * @param packet the message packet.
         * @return true if the packet was sent successfully to all receivers.
         */
        template< typename T >
        static bool send(const std::vector<T>& receivers, const Packet& packet);
        /** 
         * Sends a packaged message including additional data to multiple
         * receivers.
         *
         * @param receivers The receiving entities.
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @return true if the packet was sent successfully to all receivers.
         */
        template< typename T >
        static bool send( const std::vector<T>& receivers, Packet& packet,
                          const void* data, const uint64_t size );
        //@}

        /** 
         * Returns the state of this connection.
         * 
         * @return the state of this connection.
         */
        State getState() const { return _state; }

        /** 
         * Set the connection's description.
         * 
         * @param description the connection parameters.
         */
        void setDescription( eqBase::RefPtr<ConnectionDescription> description);

        /** @return the description for this connection. */
        eqBase::RefPtr<ConnectionDescription> getDescription();


        /** @return the notifier handle to signal that data can be read. */
#ifdef WIN32
        typedef HANDLE ReadNotifier;
        enum SelectResult
        {
            SELECT_TIMEOUT = WAIT_TIMEOUT,
            SELECT_ERROR   = WAIT_FAILED,
        };
#else
        typedef int ReadNotifier;
        enum SelectResult
        {
            SELECT_TIMEOUT = 0,
            SELECT_ERROR   = -1,
        };
#endif
        virtual ReadNotifier getReadNotifier() const { return 0; }

    protected:
        Connection();
        Connection(const Connection& conn);
        virtual ~Connection();

        /** @name Input/Output */
        //@{
        /** 
         * Read data from the connection.
         *
         * Note the the a return value of 0 is not an error condition, it means
         * that no data was pending on a non-blocking connection.
         * 
         * @param buffer the buffer for saving the message.
         * @param bytes the number of bytes to read.
         * @return the number of bytes read, or -1 upon error.
         */
        virtual int64_t read( void* buffer, const uint64_t bytes ) = 0;

        /** 
         * Write data to the connection.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to write.
         * @return the number of bytes written, or -1 upon error.
         */
        virtual int64_t write( const void* buffer, const uint64_t bytes )
            const = 0;
        //@}

        State                                 _state; //!< The connection state
        eqBase::RefPtr<ConnectionDescription> _description;

        mutable eqBase::Lock _sendLock;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const Connection* connection )
    {
        if( !connection )
        {
            os << "NULL connection";
            return os;
        }

        Connection::State state = connection->getState();
        
        os << "Connection " << (void*)connection << " type "
           << typeid(*connection).name() << " state "
           << ( state == Connection::STATE_CLOSED     ? "closed" :
                state == Connection::STATE_CONNECTING ? "connecting" :
                state == Connection::STATE_CONNECTED  ? "connected" :
                state == Connection::STATE_LISTENING  ? "listening" :
                "unknown state");
        return os;
    }

#   include "connection.ipp" // template implementation
}
#endif //EQNET_CONNECTION_H
