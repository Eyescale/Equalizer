
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_PIPE_CONNECTION_H
#define CO_PIPE_CONNECTION_H

#ifdef WIN32
#  include <co/connection.h>
#else
#  include "fdConnection.h"
#endif

#include <co/base/thread.h>

namespace co
{
    /**
     * A uni-directional pipe connection.
     *
     * The pipe connection is implemented using anonymous pipes, and can
     * therefore only be used between related threads. A PairConnection can be
     * used to create a bi-directional communication using two pipe connections.
     */
    class PipeConnection 
#ifdef WIN32
        : public Connection
#else
        : public FDConnection
#endif
    {
    public:
        /** Construct a new pipe connection. */
        CO_API PipeConnection();
        /** Destruct this pipe connection. */
        CO_API virtual ~PipeConnection();

        virtual bool connect();
        virtual void close();

#ifdef WIN32
        virtual Notifier getNotifier() const { return _dataPending; }
        bool hasData() const 
            { return WaitForSingleObject( _dataPending, 0 ) == WAIT_OBJECT_0; }
#endif

    protected:
#ifdef WIN32
        virtual void readNB( void* buffer, const uint64_t bytes );
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored );
        virtual int64_t write( const void* buffer, const uint64_t bytes );
#endif

    private:
        bool _createPipe();

#ifdef WIN32
        HANDLE _readHandle;
        HANDLE _writeHandle;
        mutable co::base::Lock _mutex;
        mutable uint64_t   _size;
        mutable HANDLE     _dataPending;
#endif
    };
}

#endif //CO_PIPE_CONNECTION_H
