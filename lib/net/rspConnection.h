
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/base/base.h>
#include <eq/net/types.h>
#include <eq/base/buffer.h> // member
#include <eq/net/connectionType.h>
#include <eq/base/rng.h>
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
        enum DatagramType 
        { 
            DATA,      // the datagram contains data
            ACK,       // ack all data
            NACK,      // annouce new data 
            ACKREQ,    // 
            NEWNODE,   // a new node is connected
            EXITNODE,  // a node is deconnected
            COUNTNODE  // send to other the number node which I have found
        };
        
        
        struct DatagramAckRequest
        {
            uint8_t    type;
            uint32_t   writerID;
            uint16_t   lastDatagramID;
            uint32_t   sequenceID;
        };
        
        struct DatagramNode
        {
            uint8_t    type;
            uint32_t   connectionID;
        };

        struct DatagramCountConnection
        {
            uint8_t     type;
            uint32_t    clientID;
            uint8_t     nbClient;
        };

        struct RepeatID
        {
            uint16_t    start;
            uint16_t    end;
        };
        struct DatagramNack
        {
        
            uint8_t     type;
            uint32_t    readerID;    // ID of the connection reader
            uint32_t    writerID;    // ID of the connection writer
            uint32_t    sequenceID;  // the last datagram in write sequence
            uint8_t     countRepeatID;
        };

        struct DatagramAck
        {
            uint8_t     type;
            uint32_t    readerID;
            uint32_t    writerID;
            uint32_t    sequenceID;

        };

        struct DatagramData
        {
            uint8_t     type;
            uint32_t    writerID;
            uint16_t    dataID;
            uint32_t    sequenceID;
            uint16_t    length;
        };
        
        struct DataReceive
        {
            uint32_t    sequenceID;
            bool        ackSend;
            bool        allRead;
            uint64_t    posRead;
            uint64_t    totalSize;
            eq::base::Buffer< bool > boolBuffer;
            eq::base::Bufferb        dataBuffer;
        };

        struct WriteDatagramData
        {
            DatagramData    header;
            const void*     data;
        };

        
        void close();
        bool connect();
        virtual void acceptNB();
        virtual ConnectionPtr acceptSync();
        void readNB( void* buffer, const uint64_t bytes );
        int64_t readSync( void* buffer, const uint64_t bytes );
        int64_t write( const void* buffer, const uint64_t bytes );
#ifdef WIN32
        /** @sa Connection::getNotifier */
        virtual Notifier getNotifier() const 
                   { return _hEvent; }
#endif
    
    private:
        /* managing RSP protocole directly on the udp connection */
        class Thread : public eq::base::Thread
        {
        public:
            Thread( RSPConnection* connection )
                : _connection( connection ){}
            virtual ~Thread(){}

        protected:
              virtual void* run();
        private:
            RSPConnection* _connection;
        };

        uint32_t _getID(){ return _myID; }
        /* get the number valid connection in the multicast network */
        uint32_t _getCountConnection()
           { return _childrensConnection.size(); }
        
        int64_t _readSync( DataReceive* receive, 
                           void* buffer, 
                           const uint64_t bytes  );
        DataReceive* _findReceiverRead();
        void _sendDatagram( const uint64_t idDatagram );
        void _sendAckRequest();
        /* using directly by the thread to manage RSP */
        void _read( );

        /* using directly by the thread to wait on event from udp connection */
        void _run();

        /* add a new connection detected in the network multicast */
        void _addNewConnection( uint64_t id );
        
        void _initAIOAccept();
        void _exitAIOAccept();
        void _initAIORead();
        void _exitAIORead();

        // for read one datagram from udp Connection
        eq::base::Bufferb _bufRead;

        // a link for all connection in the multicast network 
        std::vector< RSPConnection* > _childrensConnection;
        // The buffer used by the read function in udp socket 
        std::vector< DataReceive* >_buffer;
        // number connection accepted by server RSP 
        uint8_t _countAcceptChildren;
        
        // The buffer used by the write function in udp socket
        eq::base::Bufferb _sendBuffer;

        //int _indexRead;
        DataReceive* _currentReadSync;
        uint32_t     _myID;
        
        eq::base::RNG _rng;

        uint32_t _writerID;
        uint8_t  _countTimeOut;
        bool     _ackReceive;
#ifdef WIN32
        HANDLE _hEvent;
        HANDLE _writeEndEvent;
#else
        pollfd _hEvent;
        pollfd _writeEndEvent;
        pollfd _udpEvent;
        PipeConnection* _selfPipeWriteEventEnd;
        PipeConnection* _selfPipeHEvent;

        /** The buffer to receive commands from Event. */
        uint8_t _selfCommand;
#endif
        uint32_t       _countNbAckInWrite;
        Thread*        _thread;
        UDPConnection* _connection;
        base::Lock     _mutexConnection;
        base::Lock     _mutexRead;
        RSPConnection* _parentConnection;
        uint64_t       _maxLengthDatagramData;
        
        // buffer for read from udp Connection
        DataReceive* bufferReceive;
        
        int64_t     _lastSequenceIDAck;
        // write property part
        const char* _dataSend;
        uint64_t    _lengthDataSend;
        uint32_t    _numberDatagram;
        uint64_t    _timeEvent;
        uint32_t    _sequenceIDWrite;

        DataReceive* _findReceiverWithSequenceID( 
                                    const uint32_t sequenceID ) const;
        RSPConnection* _findConnectionWithWriterID( 
                                    const uint32_t writerID ) const;
        
        void sendNackDatagram ( const uint32_t  toWriterID,
                                const uint32_t  sequenceID,
                                const uint8_t   countNack,
                                const RepeatID* repeatID   ) const;
        
        CHECK_THREAD_DECLARE( _recvThread );
 
    };
}
}

#endif //EQNET_RSPCONNECTION_H
