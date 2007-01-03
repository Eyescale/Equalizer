
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include "packets.h"

#include <eq/base/base.h>
#include <eq/base/lock.h>
#include <eq/base/referenced.h>
#include <eq/base/refPtr.h>
#include <eq/base/scopedMutex.h>

#include <poll.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/param.h>  // for MAX()
#include <unistd.h>
#include <vector>

namespace eqNet
{
    class ConnectionDescription;

#   define EQ_DEFAULT_PORT (4242 + getuid())

    /**
     * A base class to provide communication to other hosts.
     */
    class Connection : public eqBase::Referenced
    {
    public:

        /** The supported network protocols. */
        enum Type
        {
            TYPE_TCPIP,   //!< TCP/IP networking.
            TYPE_PIPE,    //!< pipe() based bi-directional connection
            TYPE_UNI_PIPE //!< pipe() based uni-directional connection
        };

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
        static eqBase::RefPtr<Connection> create( const Type type );
        
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
         * Sends a packaged message using the connection.
         * 
         * @param packet the message packet.
         * @return the number of bytes send.
         */
        uint64_t send( const Packet &packet ) const
            {return send( &packet, packet.size); }

        /** 
         * Sends a packaged message including a string using the connection.
         * 
         * @param packet the message packet.
         * @param string the string.
         * @return the number of bytes send.
         */
        uint64_t send( Packet &packet, const std::string& string ) const
            { return send( packet, string.c_str(), string.size()+1 ); }

        /** 
         * Sends a packaged message including additional data.
         *
         * The last item of the packet has to be able to hold one item or eight
         * bytes of the data, whatever is bigger.
         * 
         * @param packet the message packet.
         * @param data the vector containing the data.
         * @return the number of bytes send.
         */
        template< typename T >
        uint64_t send( Packet &packet, const std::vector<T>& data ) const;

        /** 
         * Sends a packaged message including additional data using the
         * connection.
         * 
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @return the number of bytes send.
         */
        uint64_t send( Packet& packet, const void* data, const uint64_t size )
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

        /** 
         * Sends data using the connection.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to send.
         * @return the number of bytes send.
         */
        virtual uint64_t send( const void* buffer, const uint64_t bytes, 
                               bool isLocked = false  ) const
            {return 0;}

        /** Lock the connection, no other thread can send data. */
        void lockSend()   { _sendLock.set(); }
        /** Unlock the connection. */
        void unlockSend() { _sendLock.unset(); }

        /** 
         * Reads data from the connection.
         * 
         * @param buffer the buffer for saving the message.
         * @param bytes the number of bytes to read.
         * @return the number of bytes received.
         */
        virtual uint64_t recv( void* buffer, const uint64_t bytes )
            {return 0;}
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

        virtual int getReadFD() const { return -1; }

    protected:
        Connection();
        Connection(const Connection& conn);
        virtual ~Connection();

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
