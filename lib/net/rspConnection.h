
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

        virtual bool listen() { return connect(); }
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
        };

        struct ClientRSP
        {
            uint32_t writerId;
            uint64_t posRead;
            uint64_t totalSize;
            uint32_t idSequence;
            uint32_t lastidSequenceAck;
            uint8_t  countTimeOut;
            bool ackSend;
            bool allRead;
            bool ackReceive;  // using by a writer to know which 
                              // reader has make an ack
            // for know exactly which datagram has not been receive
            eq::base::Buffer< bool >   boolBuffer;
            eq::base::Bufferb dataBuffer;

            // maybe time last read for detect timeout
        };
        
        struct WriteDatagramData
        {
            DatagramData    header;
            const void* data;
        };

        
        void close();
        bool connect();
        virtual void acceptNB() { } 
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
        std::vector< ClientRSP* > clientsRSP;

        uint32_t _getID(){ return _myID; }
        uint32_t _getCountConnection(){ return clientsRSP.size(); }
        int64_t _readSync( void* buffer, const uint64_t bytes );
        void _updateEvent();
        void _sendDatagram( const uint64_t idDatagram );
        void _read( );
        void _run();
        void _addNewConnection( uint64_t id );
        eq::base::Bufferb _bufRead;
        eq::base::Buffer< WriteDatagramData > _datagramsData;
        eq::base::Buffer< uint32_t > _lengthsData;
        int _indexRead;
        uint32_t      _myID;
        uint32_t      _idSequenceWrite;
        eq::base::RNG _rng;
#ifdef WIN32
        HANDLE _hEvent;
        HANDLE _writeEndEvent;
#else
        pollfd _hEvent;
        pollfd _writeEndEvent;
#endif
        uint32_t _countNbAckInWrite;
        Thread* _thread;
        UDPConnection* _connection;
        base::Lock _mutexConnection;

    };
}
}

#endif //EQNET_RSPCONNECTION_H
