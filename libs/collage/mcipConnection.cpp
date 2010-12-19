
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

#include "mcipConnection.h"

#include "connectionDescription.h"
#include "rspConnection.h"

#ifdef EQ_PGM
#  include "pgmConnection.h"
#endif

namespace co
{

MCIPConnection::MCIPConnection()
{
    _description->type = CONNECTIONTYPE_MCIP;
    _description->bandwidth = 102400;
}

MCIPConnection::~MCIPConnection()
{
}

bool MCIPConnection::connect()
{
#ifdef EQ_PGM
    _description->type = CONNECTIONTYPE_PGM;

    _impl = new PGMConnection();
    _impl->addListener( this );

    // TODO: Bandwidth is set to 100MB/s, but PGM should use less
    _impl->setDescription( _description );
    if( _impl->connect( ))
        return true;
    // else

    _impl->removeListener( this );
#endif
#ifdef EQ_USE_BOOST
    _description->type = CONNECTIONTYPE_RSP;
    _impl = new RSPConnection();
    _impl->addListener( this );

    _impl->setDescription( _description );
    if( _impl->connect( ))
        return true;

    _description->type = CONNECTIONTYPE_MCIP;    
    _impl->removeListener( this );
    _impl = 0;
#endif

    return false;
}

bool MCIPConnection::listen()
{
#ifdef EQ_PGM
    _description->type = CONNECTIONTYPE_PGM;

    _impl = new PGMConnection();
    _impl->addListener( this );
    _impl->setDescription( _description );
    if( _impl->listen( ))
        return true;
    // else

    _impl->removeListener( this );
#endif
#ifdef EQ_USE_BOOST
    _description->type = CONNECTIONTYPE_RSP;    
    _impl = new RSPConnection();
    _impl->addListener( this );
    _impl->setDescription( _description );
    if( _impl->listen( ))
        return true;
    // else

    _description->type = CONNECTIONTYPE_MCIP;    
    _impl->removeListener( this );
    _impl = 0;
#endif
    return false;
}

void MCIPConnection::close()
{
    if( !_impl )
        return;

    _impl->close();
    _impl->removeListener( this );
    _impl = 0;
}

void MCIPConnection::acceptNB()
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        _impl->acceptNB();
}

ConnectionPtr MCIPConnection::acceptSync()
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->acceptSync();
    // else
    return 0;
}

void MCIPConnection::readNB( void* buffer, const uint64_t bytes )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        _impl->readNB( buffer, bytes );
}

int64_t MCIPConnection::readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->readSync( buffer, bytes, ignored );
    // else
    return -1;
}

Connection::Notifier MCIPConnection::getNotifier() const
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->getNotifier();
    // else
    return 0;
}

int64_t MCIPConnection::write( const void* buffer, const uint64_t bytes )
{
    EQASSERT( _impl.isValid( ));
    if( _impl.isValid( ))
        return _impl->write( buffer, bytes );
    // else
    return -1;
}

void MCIPConnection::notifyStateChanged( Connection* connection )
{
    EQASSERT( _impl == connection );
    if( _state == connection->getState( ))
        return;

    _state = connection->getState();
    _fireStateChanged();
}

}
