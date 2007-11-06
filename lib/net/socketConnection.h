
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKETCONNECTION_H
#define EQNET_SOCKETCONNECTION_H

#include <eq/base/base.h>
#ifdef WIN32
#  include <eq/net/connection.h>
#  ifndef MAXPATHLEN
#    define MAXPATHLEN 1024
#  endif
#else
#  include <eq/net/fdConnection.h>
#  include <netinet/in.h>
#endif


namespace eqNet
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
        virtual eqBase::RefPtr<Connection> accept();

        virtual void close();

        uint16_t getPort() const;

#ifdef WIN32
        virtual ReadNotifier getReadNotifier();
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
        bool _startAccept();
        bool _createAcceptSocket();
        bool _createReadEvent();
        
        // overlapped data structures
        OVERLAPPED _overlapped;
        bool       _overlappedPending;
        union
        {
            uint64_t      _overlappedBuffer;
            struct
            {
                void*     _overlappedAcceptData;
                SOCKET    _overlappedSocket;
            };
        };

        union
        {
            SOCKET _readFD;
            SOCKET _writeFD;
        };

        CHECK_THREAD_DECLARE( _recvThread );
#endif
    };
}

#endif //EQNET_SOCKETCONNECTION_H
