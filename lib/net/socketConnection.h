
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
        virtual void acceptNB();
        virtual ConnectionPtr acceptSync();

        virtual void close();

        uint16_t getPort() const;

#ifdef WIN32
        virtual Notifier getNotifier() const;
#endif

    protected:
        virtual ~SocketConnection();

        void _initAIOAccept();
        void _exitAIOAccept();
        void _initAIORead();
        void _exitAIORead();

#ifdef WIN32
        virtual void readNB( void* buffer, const uint64_t bytes );
        virtual int64_t readSync( void* buffer, const uint64_t bytes );
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
        union
        {
            SOCKET _readFD;
            SOCKET _writeFD;
        };

        // overlapped data structures
        OVERLAPPED _overlapped;
        void*      _overlappedAcceptData;
        SOCKET     _overlappedSocket;

        CHECK_THREAD_DECLARE( _recvThread );
#endif
    };
}
}

#endif //EQNET_SOCKETCONNECTION_H
