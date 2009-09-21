
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "ipmcConnection.h"

#ifdef EQ_PGM
#  include "pgmConnection.h"
#endif

namespace eq
{
namespace net
{

IPMCConnection::IPMCConnection()
{
    _description =  new ConnectionDescription;
    _description->type = CONNECTIONTYPE_IPMC;
    _description->bandwidth = 102400;
}

IPMCConnection::~IPMCConnection()
{
}

bool IPMCConnection::connect()
{
#ifdef EQ_PGM
    _impl = new PGMConnection();
    _impl->setDescription( _description );
    if( _impl->connect( ))
        return true;
    // else

    _impl = 0;
#endif
    // TODO: implement and try UDP-based reliable multicast

    return false;
}

bool IPMCConnection::listen()
{
#ifdef EQ_PGM
    _impl = new PGMConnection();
    _impl->setDescription( _description );
    if( _impl->listen( ))
        return true;
    // else

    _impl = 0;
#endif
    // TODO: implement and try UDP-based reliable multicast
    return false;
}

void IPMCConnection::close()
{
    if( !_impl )
        return;

    _impl->close();
    _impl = 0;
}

void IPMCConnection::acceptNB()
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        _impl->acceptNB();
}

ConnectionPtr IPMCConnection::acceptSync()
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->acceptSync();
    // else
    return 0;
}

void IPMCConnection::readNB( void* buffer, const uint64_t bytes )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        _impl->readNB( buffer, bytes );
}

int64_t IPMCConnection::readSync( void* buffer, const uint64_t bytes )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->readSync( buffer, bytes );
    // else
    return -1;
}

Connection::Notifier IPMCConnection::getNotifier() const
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->getNotifier();
    // else
    return 0;
}

int64_t IPMCConnection::write( const void* buffer, const uint64_t bytes )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->write( buffer, bytes );
    // else
    return -1;
}

}
}
