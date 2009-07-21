
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include <eq/net/connectionDescription.h> // member
#include <eq/net/packets.h>               // used in inline method
#include <eq/net/types.h>                 // ConnectionVector type

#include <eq/base/base.h>
#include <eq/base/refPtr.h>
#include <eq/base/referenced.h>
#include <eq/base/scopedMutex.h>
#include <eq/base/lock.h>

#include <sys/types.h>
#include <string.h>
#include <vector>

#ifdef WIN32_API
#  include <malloc.h>     // for alloca()
#endif

#ifdef WIN32
#  define EQ_DEFAULT_PORT (4242)
#else
#  define EQ_DEFAULT_PORT (4242 + getuid())
#endif

namespace eq
{
namespace net
{
    class ConnectionListener;
    enum ConnectionType;

    /**
     * An interface definition for communication between hosts.
     *
     * Connections are stream-oriented point-to-point communications. The
     * parameters of a Connection are described in a ConnectionDescription,
     * which is used in listen() and connect(). A Connection has a State, which
     * changes when calling listen(), connect() or close(), or whenever the
     * underlying connection is closed by the operating system.
     *
     * The Connection class defines the interface for connections, various
     * derived classes implement it for various low-level communication
     * protocols, e.g., SocketConnection for TCP/IP. An implementation may not
     * implement all the functionality defined in this interface.
     *
     * The Connection is used reference-counted in eq::net, since it has
     * multiple owners, such as the ConnectionSet and Node.
     */
    class Connection : public base::Referenced, public base::NonCopyable
    {
    public:
        enum State //! The current state of the Connection
        {
            STATE_CLOSED,     //!< Closed, initial state
            STATE_CONNECTING, //!< A connect() or listen() is in progress
            STATE_CONNECTED,  //!< The connection has been connected and is open
            STATE_LISTENING   //!< The connection is listening for connects
        };

        /** 
         * Create a new connection.
         *
         * This factory method creates a new concrete connection for the
         * requested type. The description is set on the created Connection.
         * 
         * @param description the connection parameters.
         * @return the connection.
         */
        EQ_EXPORT static ConnectionPtr create( ConnectionDescriptionPtr 
                                                   description );

        /** @name Data Access */
        //@{
        /** @return the State of this connection. */
        State getState() const { return _state; }

        /** @return true if the connection is in the closed state. */
        bool isClosed() const { return _state == STATE_CLOSED; }

        /** @return true if the connection is in the connected state. */
        bool isConnected() const { return _state == STATE_CONNECTED; }

        /** @return true if the connection is in the listening state. */
        bool isListening() const { return _state == STATE_LISTENING; }

        /** 
         * Set the connection parameter description.
         * 
         * @param description the connection parameters.
         */
        EQ_EXPORT void setDescription( ConnectionDescriptionPtr description );

        /** @return the description for this connection. */
        EQ_EXPORT ConnectionDescriptionPtr getDescription() const;
        //@}


        /** @name Connection State Changes */
        //@{
        /** 
         * Connect to the remote peer.
         *
         * The ConnectionDescription of this connection is used to identify the
         * peer's parameters.
         *
         * @return <code>true</code> if the connection was successfully
         *         connected, <code>false</code> if not.
         */
        virtual bool connect() { return false; }
        
        /** 
         * Put the connection into the listening state.
         *
         * The ConnectionDescription of this connection is used to identify the
         * listening parameters.
         *
         * @return <code>true</code> if the connection is listening for new
         *         incoming connections, <code>false</code> if not.
         */
        virtual bool listen() { return false; }

        /** 
         * Close a connected or listening connection.
         */
        virtual void close(){};
        //@}

        /** @name Listener Interface */
        //@{
        /** Add a listener for connection state changes. */
        void addListener( ConnectionListener* listener );

        /** Remove a listener for connection state changes. */
        void removeListener( ConnectionListener* listener );
        //@}

        /** @name Asynchronous accept */
        //@{
        /** 
         * Start an accept operation.
         * 
         * This method returns immediately. The Notifier will signal a new
         * connection request, upon which acceptSync() should be used to finish
         * the accept operation.
         * 
         * @sa acceptSync()
         */
        virtual void acceptNB() { EQUNIMPLEMENTED; }

        /** 
         * Complete an accept operation.
         *
         * @return the new connection, 0 on error.
         */        
        virtual ConnectionPtr acceptSync()
            { EQUNIMPLEMENTED; return 0; }
        //@}


        /** @name Asynchronous read */
        //@{
        /** 
         * Start a read operation on the connection.
         *
         * This function returns immediately. The Notifier will signal data
         * availability, upon which recvSync() should be used to finish the
         * operation.
         * 
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @sa recvSync()
         */
        EQ_EXPORT void recvNB( void* buffer, const uint64_t bytes );

        /** 
         * Finish reading data from the connection.
         * 
         * This function may block even if data availability was signaled, i.e.,
         * when only a part of the data requested has been received.  The buffer
         * and bytes return value pointers can be 0. This method uses readNB()
         * and readSync() to fill a buffer, potentially by using multiple reads.
         *
         * @param buffer return value, the buffer pointer passed to recvNB().
         * @param bytes return value, the number of bytes read.
         * @return true if all requested data has been read, false otherwise.
         */
        EQ_EXPORT bool recvSync( void** buffer, uint64_t* bytes );

        void getRecvData( void** buffer, uint64_t* bytes )
            { *buffer = _aioBuffer; *bytes = _aioBytes; }

        /** 
         * Start a read operation on the connection.
         *
         * This method is the low-level counterpart to recvNB().
         *
         * This function returns immediately. The operation's Notifier will
         * signal data availability, upon which readSync() should be used to
         * finish the operation.
         * 
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @sa readSync()
         */
        virtual void readNB( void* buffer, const uint64_t bytes ) = 0;

        /** 
         * Finish reading data from the connection.
         *
         * This method is the low-level counterpart to recvSync().
         * It may return with a partial read.
         * 
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @return the number of bytes read, or -1 upon error.
         */
        virtual int64_t readSync( void* buffer, const uint64_t bytes ) = 0;
        //@}

        /** @name Synchronous write to the connection */
        //@{
        /** 
         * Send data using the connection.
         *
         * A send may be performed using multiple write() operations. For
         * thread-safe sending from multiple threads it is therefore crucial to
         * protect the send() operation internally. If the connection is not
         * already locked externally, it will use an internal mutex.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to send.
         * @param isLocked true if the connection is locked externally.
         * @return true if all data has been read, false if not.
         * @sa lockSend(), unlockSend()
         */
        EQ_EXPORT bool send( const void* buffer, const uint64_t bytes, 
                             const bool isLocked = false );

        /** Lock the connection, no other thread can send data. */
        void lockSend() const   { _sendLock.set(); }
        /** Unlock the connection. */
        void unlockSend() const { _sendLock.unset(); }
            
        /** 
         * Sends a packaged message using the connection.
         * 
         * @param packet the message packet.
         * @return true if all data has been read, false if not.
         */
        bool send( const Packet& packet )
            { return send( &packet, packet.size); }

        /** 
         * Sends a packaged message including a string using the connection.
         * 
         * The packet has to define a 8-byte-aligned, 8-char string at the end
         * of the packet. When the packet is sent the whole string is appended
         * to the packet, so that the receiver has to do nothing special to
         * receive and use the full packet.
         *
         * @param packet the message packet.
         * @param string the string.
         * @return true if all data has been read, false if not.
         */
        bool send( Packet& packet, const std::string& string )
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
        bool send( Packet& packet, const std::vector<T>& data );

        /** 
         * Sends a packaged message including additional data using the
         * connection.
         *
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @return true if all data has been read, false if not.
         */
        EQ_EXPORT bool send( Packet& packet, const void* data,
                             const uint64_t size );

        /** 
         * Sends a packaged message to multiple connections.
         *
         * @param connections The connections.
         * @param packet      the message packet.
         * @param isLocked true if the connection is locked externally.
         * @return true if the packet was sent successfully to all connections.
         */
        static bool send( const ConnectionVector& connections,
                          const Packet& packet, const bool isLocked = false );
        /** 
         * Sends a packaged message including additional data to multiple
         * connections.
         *
         * @param connections The connections.
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @param isLocked true if the connection is locked externally.
         * @return true if the packet was sent successfully to all receivers.
         */
        static bool send( const ConnectionVector& connections, Packet& packet,
                          const void* data, const uint64_t size,
                          const bool isLocked = false );
        //@}

        /**
         * The Notifier used by the ConnectionSet to dedect readyness of a
         * Connection.
         */
#ifdef WIN32
        typedef HANDLE Notifier;
#else
        typedef int Notifier;
#endif
        /** @return the notifier signalling events on the connection. */
        virtual Notifier getNotifier() const { return 0; }

    protected:
        Connection();
        virtual ~Connection();

        void _fireStateChanged();

        /** @name Input/Output */
        //@{
        /** 
         * Write data to the connection.
         * 
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to write.
         * @return the number of bytes written, or -1 upon error.
         */
        virtual int64_t write( const void* buffer, const uint64_t bytes ) = 0;
        //@}

        State                    _state; //!< The connection state
        ConnectionDescriptionPtr _description; //!< The connection parameters

        /** The lock used to protect multiple write calls. */
        mutable base::Lock _sendLock;

    private:
        void*         _aioBuffer;
        uint64_t      _aioBytes;

        /** The listeners on state changes */
        std::vector< ConnectionListener* > _listeners;

        friend class PairConnection; //!< for access to read/write
    };

    std::ostream& operator << ( std::ostream&, const Connection* );

#   include "connection.ipp" // template implementation

}
}
#endif //EQNET_CONNECTION_H
