
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *                     Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQNET_RSPCONNECTION_H
#define EQNET_RSPCONNECTION_H
#include <pthread.h>
#include <eq/base/base.h>
#include <eq/base/buffer.h> // member
#include <eq/base/mtQueue.h>
#include <eq/base/rng.h>
#include <eq/net/types.h>
#include <eq/net/connectionType.h>

#include "connectionSet.h"
#include "udpConnection.h"
#include "fdConnection.h"
#ifndef WIN32
#  include <poll.h>
#  include "pipeConnection.h"
#endif
namespace eq
{
namespace net
{

    class ConnectionDescription;
    class RSPConnection;
    /** A rsp connection (Attn: only multicast usage implemented). */
    class RSPConnection
#ifdef WIN32
        : public Connection
#else
        : public FDConnection
#endif
    {

    public:
        /** Create a new rsp-based connection. */
        RSPConnection();
        virtual ~RSPConnection();

        virtual bool listen();
        typedef uint16_t ID;
        void close();
        bool connect(){ return listen(); }

        virtual void acceptNB(){ EQASSERT( _state == STATE_LISTENING ); }

        virtual ConnectionPtr acceptSync();
        void readNB( void* buffer, const uint64_t bytes ){/* NOP */}
        int64_t readSync( void* buffer, const uint64_t bytes );
        int64_t write( const void* buffer, const uint64_t bytes );

        int64_t getSendRate() const { return _connection->getSendRate(); }
        uint32_t getID() const { return _id;}
        
#ifdef WIN32
        /** @sa Connection::getNotifier */
        virtual Notifier getNotifier() const 
                   { return _hEvent; }
#endif
    
    private:

        typedef base::RefPtr< UDPConnection > UDPConnectionPtr;
        typedef base::RefPtr< RSPConnection > RSPConnectionPtr;
        
        /* managing RSP protocole directly on the udp connection */
        class Thread : public eq::base::Thread
        {
        public: 
            Thread( RSPConnectionPtr connection )
                : _connection( connection ){}
            virtual ~Thread(){ _connection = 0; }
            virtual bool init(){ return _connection->_acceptID() && 
                                        _connection->_initReadThread(); }
        protected:
            
            virtual void* run();
        private:
            RSPConnectionPtr _connection;
        };

        enum DatagramType 
        { 
            // exchange datagram during data send
            DATA,      // the datagram contains data
            ACKREQ,    // ask for ack from all readers
            NACK,      // negative ack, request missing packets
            ACK,       // positive ack all data
            ID_HELLO, // a new node is connected.
            ID_DENY,
            ID_CONFIRM,
            ID_EXIT,  // a node is disconnected
            COUNTNODE  // send to other the number node which I have found
        };
        
        struct DatagramAckRequest
        {
            uint16_t    type;
            ID   writerID;
            uint16_t lastDatagramID;
            uint16_t sequenceID;
        };
        
        struct DatagramNode
        {
            uint16_t            type;
            ID   connectionID;
        };

        struct DatagramCountConnection
        {
            uint16_t             type;
            ID    clientID;
            uint16_t             nbClient;
        };

        struct DatagramNack
        {
            uint16_t       type;
            ID             readerID;    // ID of the connection reader
            ID             writerID;    // ID of the connection writer
            uint16_t       sequenceID;  // last datagram in write sequence
            uint16_t       count; // number of NACK requests
        };

        struct DatagramAck
        {
            uint16_t        type;
            ID              readerID;
            ID              writerID;
            uint16_t        sequenceID;
        };

        struct DatagramData
        {
            uint16_t    type;
            uint32_t    writeSeqID;
            uint32_t    dataIDlength;
        };
        
        struct InBuffer
        {
            InBuffer() { reset(); }
            void reset();
            uint32_t    sequenceID;
            eq::base::Monitor< bool >  ackSend;
            eq::base::Monitor< bool >  allRead;
            uint64_t    readPos;
            base::Bufferb got;
            base::Bufferb data;
        };

        struct WriteDatagramData
        {
            DatagramData    header;
            const void*     data;
        };

        struct RepeatRequest
        {
            enum Type
            {
                DONE,
                ACKREQ,
                NACK
            };

            RepeatRequest() : type( NACK ), start( 0 ), end( 0 ) {}
            
            RepeatRequest( const Type& t ) 
                : type( t ), start( 0 ), end( 0 ) {}

            Type type;
            uint32_t start;
            uint32_t end;
        };
        
        // Buffer for one datagram from our UDP connection
        eq::base::Bufferb _readBuffer;

        // Buffer to send one NACK packet.
        eq::base::Bufferb _nackBuffer;

        // a link for all connection in the multicast network 
        std::vector< RSPConnectionPtr > _children;

        // number connection accepted by server RSP 
        base::a_int32_t _countAcceptChildren;
        
        // The buffer used by the write function in udp socket
        eq::base::Bufferb _sendBuffer;

        eq::base::MTQueue< RepeatRequest > _repeatQueue;

        eq::base::RNG         _rng;
        ID _id; //!< The identifier used to demultiplex multipe writers
        uint32_t _shiftedID; //!< Shifted _id for easier packaging
        
        int32_t  _timeouts;
        uint16_t _ackReceived; // sequence ID of last received ack for child

#ifdef WIN32
        HANDLE _hEvent;
#else
        pollfd _hEvent;
        PipeConnection* _selfPipeHEvent;

        /** The buffer to receive commands from Event. */
        uint8_t _selfCommand;
#endif
        ConnectionSet    _connectionSet;
        bool             _writing;
        uint32_t         _numWriteAcks;
        Thread*          _thread;
        UDPConnectionPtr _connection;
        base::Lock       _mutexConnection;
        base::Lock       _mutexEvent;
        RSPConnectionPtr _parent;
        int64_t    _lastSequenceIDAck;
        
        // Current read buffer
        std::vector< InBuffer* > _inBuffers;
        
        InBuffer*  _recvBuffer;
        size_t     _recvBufferIndex; //!< current partial recv thread buffer

        size_t _readBufferIndex; //!< current filled app read buffer

        // write property part
        uint32_t _nDatagrams;
        uint16_t _sequenceID;

        static uint32_t _payloadSize;
        static size_t   _bufferSize;
        static uint32_t _maxNAck;

        ID _buildNewID();
        
        int64_t _readSync( InBuffer* readBuffer, void* buffer, 
                           const uint64_t bytes  );
        bool _acceptID();
        bool _handleAcceptID();
        /* using directly by the thread to manage RSP */
        bool _handleData( );
        bool _handleDataDatagram( const DatagramData* datagram );
        bool _handleAck( const DatagramAck* ack );
        bool _handleNack( const DatagramNack* nack );
        bool _handleAckRequest( const DatagramAckRequest* ackRequest );

        /** @return true if we knew the correct number of connections. */
        bool _handleCountNode();

        /** Initialize the reader thread */
        bool _initReadThread();

        /* Run the reader thread */
        void _runReadThread();
        
        bool _handleInitData();
        
        void _initAIOAccept(){ _initAIORead(); }
        void _exitAIOAccept(){ _exitAIORead(); }
        void _initAIORead();
        void _exitAIORead();

        /** find the receiver corresponding to the sequenceID */
        InBuffer* _findReceiverWithSequenceID(const uint16_t sequenceID )
            const;
        
        /** find the connection corresponding to the writeID */
        RSPConnectionPtr _findConnectionWithWriterID( 
                                    const ID writerID );
        /** determine if the sequenceID and writerID correspond to 
            the current sequence which will being write*/
        bool _isCurrentSequence( const uint16_t sequenceID, const ID writer );
        
        /** Analyze the current error and adapt the send rate */
        void _adaptSendRate( const uint64_t errors );

        /** format and send a datagram count node */
        void _sendDatagramCountNode();

        int64_t _handleRepeat( const uint8_t* data, const uint64_t size );
        void _addRepeat( const uint32_t* repeatIDs, uint32_t size );

        /** format and send a data packet*/
        void _sendDatagram( const uint8_t* data, const uint64_t size,
                            const uint32_t writSeqID, 
                            const uint16_t datagramID );

        /** format and send an ack request*/
        void _sendAckRequest( );

        /** format and send a positive ack */
        void _sendAck( const ID writerID, const uint16_t sequenceID );
        
        /** format and send a negative ack*/ 
        void _sendNack( const ID  toWriterID, const uint16_t  sequenceID,
                        const uint8_t countNack, const uint32_t* repeatID   );
        
        void _checkNewID( const ID id );

        /* add a new connection detected in the multicast network */
        bool _addNewConnection( const ID id );
        bool _acceptRemoveConnection( const ID id );

        void _setEvent();
        void _resetEvent();

        CHECK_THREAD_DECLARE( _recvThread );
 
    };

    std::ostream& operator << ( std::ostream&, const RSPConnection& );
}
}

#endif //EQNET_RSPCONNECTION_H
