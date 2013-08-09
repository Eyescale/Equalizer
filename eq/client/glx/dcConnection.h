
/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef EQ_DC_CONNECTION_H
#define EQ_DC_CONNECTION_H

#include "../dcProxy.h"
#include <co/connection.h>


namespace eq
{
namespace glx
{
    /**
     * @internal
     * A DisplayCluster connection wrapper.
     *
     * This class is used to monitor multiple DisplayCluster connections for
     * events using a co::ConnectionSet.
     */
    class DcConnection : public co::Connection
    {
    public:
        DcConnection( DcProxy* dcProxy )
                : _dcProxy( dcProxy ) { _setState( STATE_CONNECTED ); }

        virtual ~DcConnection() { _setState( STATE_CLOSED ); }

        virtual Notifier getNotifier() const
            { return _dcProxy->getSocketDescriptor(); }

        const DcProxy* getDcProxy() const { return _dcProxy; }

    protected:
        virtual void readNB( void*, const uint64_t ) { LBDONTCALL; }
        virtual int64_t readSync( void*, const uint64_t, const bool )
            { LBDONTCALL; return -1; }
        virtual int64_t write( const void*, const uint64_t )
            { LBDONTCALL; return -1; }

    private:
        DcProxy* const _dcProxy;
    };
}
}

#endif // EQ_DC_CONNECTION_H
