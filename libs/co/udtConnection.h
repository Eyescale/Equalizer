// -*- mode: c++ -*-
/* Copyright (c) 2012, Computer Integration & Programming Solutions, Corp. and
 *                     United States Naval Research Laboratory
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
#pragma once

#include <co/connection.h>
#include <co/base/monitor.h>

#include <udt.h>

namespace co
{
/**
 * A UDT connection implementation.
 *
 * "UDT is a reliable UDP based application level data transport protocol for
 * distributed data intensive applications over wide area high-speed networks."
 *
 * See: http://udt.sourceforge.net
 *
 * Notes:
 *
 * - UDT::connect "takes at least one round trip to finish".
 *
 * - Abrupt disconnects can take a while to timeout.
 *
 */
class UDTConnection : public Connection
{
public:
    UDTConnection( );

    virtual bool connect( );
    virtual bool listen( );
    virtual void close( );

    virtual void acceptNB( );
    virtual ConnectionPtr acceptSync( );

public:
    virtual void    readNB  ( void* buffer, const uint64_t bytes );
    virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                                  const bool ignored );
    virtual int64_t write   ( const void* buffer, const uint64_t bytes );

public:
    virtual Notifier getNotifier( ) const { return _notifier; };

protected:
    virtual ~UDTConnection( );

private:
    bool initialize( );
    void wake( );

    bool tuneSocket( );
    bool setSockOpt( UDT::SOCKOPT optname, const void *optval, int optlen );

private:
    UDTSOCKET _udt;

    Notifier _notifier;

    class UDTConnectionThread;
    UDTConnectionThread *_poller;

    base::Monitor<bool> _app_block;
    base::Lock _app_mutex; // Blocks connection thread until app handles event
}; // UDTConnection
} // namespace co
