
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
            uint16_t   lastDatagramid;
            uint32_t   idSequence;
        };
        
        struct DatagramNode
        {
            uint8_t    type;
            uint32_t   idConnection;
        };

        struct DatagramCountConnection
        {
            uint8_t     type;
            uint32_t    idClient;
            uint8_t     nbClient;
        };

        struct DatagramNack
        {
        
            uint8_t     type;
            uint32_t    readerId;    // ID of the connection reader
            uint32_t    writerId;    // ID of the connection writer
            uint16_t    idDataStart;
            uint16_t    idDataEnd;
            uint32_t    idSequence;  // the last datagram in write sequence
        };

        struct DatagramAck
        {
            uint8_t     type;
            uint32_t    readerID;
            uint32_t    writerID;
            uint32_t    idSequence;

        };

        struct DatagramData
        {
            uint8_t     type;
            uint32_t    idwriter;
            uint16_t    idData;
            uint32_t    idSequence;
            uint16_t    length;
        };
        
        struct DataReceive
        {
            // for know exactly which datagram has not been receive
            uint32_t _idSequence;
            bool _ackSend;
            bool _allRead;
            
            uint64_t _posRead;
            uint64_t _totalSize;
            
            eq::base::Buffer< bool > _boolBuffer;
            eq::base::Bufferb        _dataBuffer;

        };
        struct WriteDatagramData
        {
            DatagramData    header;
            const void* data;
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
        uint32_t _getCountConnection(){ return _childrensConnection.size(); }
        int64_t _readSync( DataReceive* receive, 
                           void* buffer, 
                           const uint64_t bytes  );
        DataReceive* _findReceiverRead();
        void _sendDatagram( const uint64_t idDatagram );
        void _sendAckRequest();
        void _read( );
        void _run();
        void _addNewConnection( uint64_t id );
        
        void _initAIOAccept();
        void _exitAIOAccept();
        void _initAIORead();
        void _exitAIORead();

        // for read one datagram from udp Connection
        eq::base::Bufferb _bufRead;
        
        std::vector< RSPConnection* > _childrensConnection;
        std::vector< DataReceive* >_buffer;
        uint8_t _countAcceptChildren;
        
        eq::base::Bufferb sendBuffer;
        int _indexRead;
        DataReceive* _currentReadSync;
        uint32_t      _myID;
        
        eq::base::RNG _rng;

        uint32_t _writerId;
        uint8_t  _countTimeOut;
        bool _ackReceive;
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
        uint32_t _countNbAckInWrite;
        Thread* _thread;
        UDPConnection* _connection;
        base::Lock _mutexConnection;
        base::Lock _mutexRead;
        RSPConnection* _parentConnection;
        uint64_t _maxLengthDatagramData;
        
        // buffer for read from udp Connection
        DataReceive* bufferReceive;
        
        int64_t     _lastidSequenceAck;
        // write property part
        const char* _dataSend;
        uint64_t    _lengthDataSend;
        uint32_t    _numberDatagram;
        uint64_t    _timeEvent;
        uint32_t    _idSequenceWrite;

        CHECK_THREAD_DECLARE( _recvThread );
 
    };
}
}

#endif //EQNET_RSPCONNECTION_H
