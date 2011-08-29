
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef CO_DATAISTREAMQUEUE_H
#define CO_DATAISTREAMQUEUE_H

#include <co/types.h>

#include <co/base/mtQueue.h> // member
#include <co/base/pool.h>    // member
#include <co/base/stdExt.h>  // member
#include <co/base/thread.h>  // thread-safety check

namespace co
{
    /** 
     * @internal
     * Manages lifecycle of DataIStreams (assembles, queues and reuses them).
     */
    class DataIStreamQueue
    {
    public:
        DataIStreamQueue();
        ~DataIStreamQueue();

        ObjectDataIStream* pop() { return _queued.pop(); }
        ObjectDataIStream* tryPop();
        ObjectDataIStream* pull( const uint128_t& version );
        void recycle( ObjectDataIStream* stream )
            { _iStreamCache.release( stream ); }

        bool addDataPacket( const uint128_t& key, Command& command );

    protected:
        typedef stde::hash_map< uint128_t, ObjectDataIStream* > PendingStreams;
        typedef PendingStreams::const_iterator PendingStreamsCIter;

        /** Not yet ready streams. */
        PendingStreams _pending;

        /** The change queue. */
        base::MTQueue< ObjectDataIStream* > _queued;

        /** Cached input streams (+decompressor) */
        base::Pool< ObjectDataIStream, true > _iStreamCache;

        EQ_TS_VAR( _thread );
    };
}

#endif // CO_DATAISTREAMQUEUE_H
