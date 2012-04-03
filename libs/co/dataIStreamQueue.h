
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

#include <lunchbox/mtQueue.h> // member
#include <lunchbox/pool.h>    // member
#include <lunchbox/stdExt.h>  // member
#include <lunchbox/thread.h>  // thread-safety check

#include "objectDataIStream.h" // pooled object

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

        bool addDataPacket( const uint128_t& key, Command& command );

        ObjectDataIStream* pop() { return _queued.pop().second; }
        ObjectDataIStream* tryPop();
        ObjectDataIStream* pull( const uint128_t& key );
        void recycle( ObjectDataIStream* stream );

    protected:
        typedef stde::hash_map< uint128_t, ObjectDataIStream* > PendingStreams;
        typedef PendingStreams::const_iterator PendingStreamsCIter;

        /** Not yet ready streams. */
        PendingStreams _pending;
        
        typedef std::pair< uint128_t, ObjectDataIStream* > QueuedStream;
        typedef std::vector< QueuedStream > QueuedStreams;

        /** The change queue. */
        lunchbox::MTQueue< QueuedStream > _queued;

        /** Cached input streams (+decompressor) */
        lunchbox::Pool< ObjectDataIStream, true > _iStreamCache;

        LB_TS_VAR( _thread );
    };
}

#endif // CO_DATAISTREAMQUEUE_H
