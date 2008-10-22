
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKETCONNECTION_H
#define EQNET_SOCKETCONNECTION_H

#include <eq/base/base.h>
#include <eq/base/buffer.h> // member

#ifdef WIN32
#  include <eq/net/connection.h>
#else
#  include <eq/net/fdConnection.h>
#  include <netinet/in.h>
#endif


namespace eq
{
namespace net
{
    /**
     * A TCP/IP-based socket connection.
     */
    class SocketConnection
#ifdef WIN32
        : public Connection
#else
        : public FDConnection
#endif
    {
    public:
        SocketConnection( const ConnectionType type );

        virtual bool connect();
        virtual bool listen();
        virtual ConnectionPtr accept();

        virtual void close();

        uint16_t getPort() const;

#ifdef WIN32
        virtual ReadNotifier getReadNotifier() const;
#endif

    protected:
        virtual ~SocketConnection();

#ifdef WIN32
        virtual int64_t read( void* buffer, const uint64_t bytes );
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const;

        typedef SOCKET Socket;
#else
        typedef int    Socket;
        enum
        {
            INVALID_SOCKET = -1
        };
#endif

    private:
        bool _createSocket();
        void _tuneSocket( const Socket fd );
        bool _parseAddress( sockaddr_in& socketAddress );

#ifdef WIN32
        void _startAccept();
        void _startReceive();
        bool _createReadEvents();
        
        // overlapped data structures
        OVERLAPPED _overlapped;
        bool       _overlappedPending;
        void*      _overlappedAcceptData;
        SOCKET     _overlappedSocket;

        enum
        {
            MIN_BUFFER_SIZE = 8,
            MAX_BUFFER_SIZE = 65536,
        };
        typedef base::Buffer< char > Buffer;

        Buffer   _pendingBuffer;
        Buffer   _receivedBuffer;
        uint64_t _receivedUsedBytes;
        HANDLE   _receivedDataEvent;

        union
        {
            SOCKET _readFD;
            SOCKET _writeFD;
        };

        CHECK_THREAD_DECLARE( _recvThread );
#endif
    };
}
}

#endif //EQNET_SOCKETCONNECTION_H
