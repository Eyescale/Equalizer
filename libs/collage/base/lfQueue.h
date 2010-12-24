
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_LFQUEUE_H
#define COBASE_LFQUEUE_H

#include <co/base/atomic.h> // member
#include <co/base/debug.h>  // used in inline method
#include <co/base/nonCopyable.h>  // base class
#include <co/base/thread.h> // thread-safety checks

#include <vector>

namespace co
{
namespace base
{
    /**
     * A thread-safe, lock-free queue with non-blocking access.
     *
     * Typically used for caches and non-blocking communication between two
     * execution threads.
     *
     * Current implementation constraints:
     * <ul>
     *   <li>One reader thread</li>
     *   <li>One writer thread</li>
     *   <li>Fixed maximum size (writes may fail)</li>
     *   <li>Not copyable</li>
     * </ul>
     */
    template< typename T > class LFQueue : public NonCopyable
    {
    public:
        /** Construct a new queue. @version 1.0 */
        LFQueue( const int32_t size )
                : _data( size + 1 ), _readPos( 0 ), _writePos( 0 ) {}

        /** Destruct this queue. @version 1.0 */
        ~LFQueue() {}

        /** @return true if the queue is empty, false otherwise. @version 1.0 */
        bool isEmpty() const { return _readPos == _writePos; }

        /** Reset (empty) the queue. @version 1.0 */
        void clear()
            {
                EQ_TS_SCOPED( _reader );
                _readPos = 0;
                _writePos = 0;
            }
            
        /**
         * Resize and reset the queue.
         * 
         * This method is not thread-safe. The queue has to be empty.
         * @version 1.0
         */
        void resize( const int32_t size )
            {
                EQASSERT( isEmpty( ));
                _readPos = 0;
                _writePos = 0;
                _data.resize( size + 1 );
            }

        /** 
         * Retrieve and pop the front element from the queue.
         *
         * @param result the front value or unmodified
         * @return true if an element was placed in result, false if the queue
         *         is empty.
         * @version 1.0
         */
        bool pop( T& result )
            {
                EQ_TS_SCOPED( _reader );
                if( _readPos == _writePos )
                    return false;
                
                result = _data[ _readPos ];
                _readPos = (_readPos + 1) % _data.size();
                return true;
            }

        /** 
         * Retrieve the front element from the queue.
         *
         * @param result the front value or unmodified
         * @return true if an element was placed in result, false if the queue
         *         is empty.
         * @version 1.0
         */
        bool getFront( T& result )
            {
                EQ_TS_SCOPED( _reader );
                if( _readPos == _writePos )
                    return false;
                
                result = _data[ _readPos ];
                return true;
            }

        /**
         * Push a new element to the back of the queue.
         *
         * @param element the element to add.
         * @return true if the element was placed, false if the queue is full
         * @version 1.0
         */
        bool push( const T& element )
            {
                EQ_TS_SCOPED( _writer );
                int32_t nextPos = (_writePos + 1) % _data.size();
                if( nextPos == _readPos )
                    return false;

                _data[ _writePos ] = element;
                _writePos = nextPos;
                return true;
            }

        /**
         * @return the maximum number of elements held by the queue.
         * @version 1.0
         */
        size_t getCapacity() const { return _data.size() - 1; }

    private:
        std::vector< T > _data;
        a_int32_t _readPos;
        a_int32_t _writePos;

        EQ_TS_VAR( _reader );
        EQ_TS_VAR( _writer );
    };

}
}
#endif // COBASE_LFQUEUE_H
