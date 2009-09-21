
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

#ifndef WIN32
#ifndef EQNET_FDCONNECTION_H
#define EQNET_FDCONNECTION_H

#include <eq/net/connection.h>

namespace eq
{
namespace net
{

    /**
     * A generic file descriptor-based connection, to be subclassed.
     */
    class EQ_EXPORT FDConnection : public Connection
    {
    public:
#ifndef WIN32
        virtual Notifier getNotifier() const { return _readFD; }
#endif

        bool hasData() const;

    protected:
        FDConnection();
        virtual ~FDConnection() {}

        virtual void readNB( void* buffer, const uint64_t bytes );
        virtual int64_t readSync( void* buffer, const uint64_t bytes );
        virtual int64_t write( const void* buffer, const uint64_t bytes );

        int   _readFD;     //!< The read file descriptor.
        int   _writeFD;    //!< The write file descriptor.

        friend inline std::ostream& operator << ( std::ostream& os, 
                                               const FDConnection* connection );
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const FDConnection* connection )
    {
        os << (Connection*)connection << " readFD " << connection->_readFD
           << " writeFD " << connection->_writeFD;
        return os;
    }
}
}

#endif //EQNET_FDCONNECTION_H
#endif // WIN32
