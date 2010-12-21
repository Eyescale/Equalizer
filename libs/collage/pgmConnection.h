
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_PGMCONNECTION_H
#define CO_PGMCONNECTION_H

#include <co/base/os.h>
#ifdef EQ_PGM

#include <co/connection.h>
#include <co/base/buffer.h> // member
#include <co/base/thread.h> // for EQ_TS_VAR

namespace co
{
    /** A PGM connection (Pragmatic General Multicast). */
    class PGMConnection : public Connection
    {
    public:
        /** Create a new PGM connection */
        PGMConnection();

        virtual bool connect();            
        virtual bool listen();             

        virtual void acceptNB();           
        virtual ConnectionPtr acceptSync();
        virtual void close();              

        /** @sa Connection::getNotifier */
        virtual Notifier getNotifier() const { return _overlapped.hEvent; }

    protected:
        virtual ~PGMConnection();

        virtual void readNB( void* buffer, const uint64_t bytes );
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored );
        virtual int64_t write( const void* buffer, const uint64_t bytes );

    private:
        void _initAIOAccept();
        void _exitAIOAccept();
        void _initAIORead();
        void _exitAIORead();

        SOCKET _initSocket( sockaddr_in address );
        bool _parseAddress( sockaddr_in& address );
        bool _parseHostname( const std::string& hostname,
                             unsigned long& address );
        uint16_t _getPort() const;

        bool _setupSendSocket();
        bool  _enableHighSpeed( SOCKET fd );
        bool  _setSendInterface();
        bool  _setSendRate();
        bool  _setSendBufferSize( const int newSize );

        bool _setupReadSocket();
        bool  _setReadInterface();
        bool  _setReadBufferSize( const int newSize );

        bool  _setFecParameters( const SOCKET fd,
                                 const int blocksize, 
                                 const int groupsize, 
                                 const int ondemand, 
                                 const int proactive );

        void _printReadStatistics();
        void _printSendStatistics();

        SOCKET _readFD;
        SOCKET _writeFD;

        // overlapped data structures
        OVERLAPPED _overlapped;
        void*      _overlappedAcceptData;
        SOCKET     _overlappedSocket;

        EQ_TS_VAR( _recvThread );
    };
}

#endif // EQ_PGM
#endif // CO_PGMCONNECTION_H
