
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_FDCONNECTION_H
#define CO_FDCONNECTION_H

#include <co/connection.h>

namespace co
{
#ifdef _WIN32
#  error FDConnection not used nor supported on Windows
#endif

    /** A generic file descriptor-based connection, to be subclassed. */
    class FDConnection : public Connection
    {
    public:
        virtual Notifier getNotifier() const { return _readFD; }

    protected:
        FDConnection();
        virtual ~FDConnection() {}

        virtual void readNB( void*, const uint64_t ) { /* NOP */ }
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored );
        virtual int64_t write( const void* buffer, const uint64_t bytes );

        int   _readFD;     //!< The read file descriptor.
        int   _writeFD;    //!< The write file descriptor.

        friend inline std::ostream& operator << ( std::ostream& os, 
                                               const FDConnection* connection );

    private:
        int _getTimeOut();

    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const FDConnection* connection )
    {
        os << (Connection*)connection << " readFD " << connection->_readFD
           << " writeFD " << connection->_writeFD;
        return os;
    }
}

#endif //EQNET_FDCONNECTION_H
