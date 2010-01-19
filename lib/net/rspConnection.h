
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/net/connectionSet.h> // member
#include <eq/net/types.h>
#include <eq/base/base.h>
#include <eq/base/buffer.h>  // member
#include <eq/base/lfQueue.h> // member
#include <eq/base/mtQueue.h> // member

#include "udpConnection.h"
#ifndef WIN32
#  include "pipeConnection.h"
#endif

namespace eq
{
namespace net
{

    class ConnectionDescription;
    class RSPConnection;
    /** A rsp connection (Attn: only multicast usage implemented). */
    class RSPConnection : public Connection
    {

    public:
        /** Create a new rsp-based connection. */
        RSPConnection();
        virtual ~RSPConnection();

        virtual bool listen();
        void close();
        bool connect(){ return listen(); }

        virtual void acceptNB(){ EQASSERT( _state == STATE_LISTENING ); }

        virtual ConnectionPtr acceptSync();
        void readNB( void* buffer, const uint64_t bytes ){/* NOP */}
        int64_t readSync( void* buffer, const uint64_t bytes );
        int64_t write( const void* buffer, const uint64_t bytes );

        int64_t getSendRate() const { return _connection->getSendRate(); }
        uint32_t getID() const { return _id; }
        
        /** @sa Connection::getNotifier */
        virtual Notifier getNotifier() const 
#ifdef WIN32
            { return _hEvent; }
#else
            { return _selfPipeHEvent->getNotifier(); }
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
            DATA,      // the datagram contains data
            ACKREQ,    // ask for ack from all readers
            NACK,      // negative ack, request missing packets
            ACK,       // positive ack all data
            ID_HELLO,  // announce a new id
            ID_DENY,   // deny the id, already used
            ID_CONFIRM,// a new node is connected
            ID_EXIT,   // a node is disconnected
            COUNTNODE  // send to other the number of nodes which I have found
        };
        
        struct DatagramAckRequest
        {
            uint16_t type;
            uint16_t writerID;
            uint16_t sequenceID;
        };
        
        struct DatagramNode
        {
            uint16_t type;
            uint16_t connectionID;
        };

        struct DatagramCountConnection
        {
            uint16_t type;
            uint16_t clientID;
            uint16_t nbClient;
        };

        struct DatagramNack
        {
            uint16_t       type;
            uint16_t       readerID;    // ID of the connection reader
            uint16_t       writerID;    // ID of the connection writer
            uint16_t       sequenceID;  // last datagram in write sequence
            uint16_t       count;       // number of NACK requests
        };

        struct DatagramAck
        {
            uint16_t        type;
            uint16_t        readerID;
            uint16_t        writerID;
            uint16_t        sequenceID;
        };

        struct DatagramData
        {
            uint16_t    type;
            uint16_t    size;
            uint16_t    writerID;
            uint16_t    sequenceID;
        };
        
        struct InBuffer
        {
            InBuffer() : sequenceID( -1 ){}
            InBuffer( const InBuffer& from ) : sequenceID( -1 ) {}

            int32_t sequenceID;
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
        
        // Read buffer for one datagram from our UDP connection
        eq::base::Bufferb _udpBuffer;

        // Buffer to send one NACK packet.
        eq::base::Bufferb _nackBuffer;

        // The buffer used by the write function in udp socket
        eq::base::Bufferb _sendBuffer;

        // a link for all connection in the multicast network 
        std::vector< RSPConnectionPtr > _children;

        // number connection accepted by server RSP 
        base::a_int32_t _countAcceptChildren;
        
        eq::base::MTQueue< RepeatRequest > _repeatQueue;

        uint16_t _id; //!< The identifier used to demultiplex multipe writers
        
        int32_t  _timeouts;
        uint16_t _ackReceived; // sequence ID of last received ack for child

#ifdef WIN32
        HANDLE _hEvent;
#else
        typedef base::RefPtr< PipeConnection > PipeConnectionPtr;
        PipeConnectionPtr _selfPipeHEvent;

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
        int32_t _lastAck;
        
        typedef std::vector< InBuffer > InBufferVector;
        InBufferVector _inBuffers;                 //!< Data buffers
        base::LFQueue< InBuffer* > _freeBuffers;   //!< Unused data buffers
        base::MTQueue< InBuffer* > _readBuffers;   //!< Ready data buffers
        InBuffer* _recvBuffer;                     //!< Receive (thread) buffer
        InBuffer* _readBuffer;                     //!< Read (app) buffer
        uint64_t _readBufferPos;                   //!< Current read index

        // write property part
        uint32_t _nDatagrams;
        uint16_t _sequenceID;

        static uint32_t _payloadSize;
        static size_t   _bufferSize;
        static uint32_t _maxNAck;

        uint16_t _buildNewID();
        
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

        /** find the connection corresponding to the identifier */
        RSPConnectionPtr _findConnection( const uint16_t id );
        /** determine if the sequenceID and writerID correspond to 
            the current sequence which will written */
        bool _isCurrentSequence( const uint16_t sequenceID, 
                                 const uint16_t writer );
        
        /** Analyze the current error and adapt the send rate */
        void _adaptSendRate( const size_t nPackets, const size_t errors );

        /** format and send a datagram count node */
        void _sendDatagramCountNode();

        void _handleRepeat( const uint8_t* data, const uint64_t size );
        void _addRepeat( const uint32_t* repeatIDs, uint32_t size );

        /** format and send a data packet*/
        void _sendDatagram( const uint8_t* data, const uint64_t size,
                            const uint16_t which );

        /** format and send an ack request*/
        void _sendAckRequest( const uint16_t sequenceID );

        /** format and send a positive ack */
        void _sendAck( const uint16_t writerID, const uint16_t sequenceID );
        
        /** format and send a negative ack*/ 
        void _sendNack( const uint16_t toWriterID, const uint16_t sequenceID,
                        const uint8_t countNack, const uint32_t* repeatID   );
        
        void _checkNewID( const uint16_t id );

        /* add a new connection detected in the multicast network */
        bool _addNewConnection( const uint16_t id );
        bool _removeConnection( const uint16_t id );

        void _setEvent();
        void _resetEvent();

        CHECK_THREAD_DECLARE( _recvThread );
 
    };

    std::ostream& operator << ( std::ostream&, const RSPConnection& );
}
}

#endif //EQNET_RSPCONNECTION_H
