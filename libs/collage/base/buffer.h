
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_BUFFER_H
#define COBASE_BUFFER_H

#include <co/base/debug.h>       // EQASSERT macro
#include <co/base/types.h>

#include <cstdlib>      // for malloc

namespace co
{
namespace base
{
    /**
     * A simple memory buffer with some helper functions.
     *
     * For bigger data (>100k) using a std::vector< uint8_t > has a high
     * overhead when resizing (>1ms). This buffer just memcpy's elements, i.e.,
     * it should only be used for PODs since the copy constructor or assignment
     * operator is not called on the copied elements. Primarily used for binary
     * data, e.g., in eq::Image. The implementation works like a pool, that is,
     * data is only released when the buffer is deleted or clear() is called.
     */
    template< typename T > class Buffer
    {
    public:
        /** Construct a new, empty buffer. @version 1.0 */
        Buffer() : _data(0), _size(0), _maxSize(0) {}

        /** Construct a new buffer of the given size. @version 1.0 */
        Buffer( const uint64_t size ) : _data(0), _size(0), _maxSize(0)
            { reset( size ); }

        /** Destruct the buffer. @version 1.0 */
        ~Buffer() { clear(); }

        /** Flush the buffer, deleting all data. @version 1.0 */
        void clear() 
            { if( _data ) free( _data ); _data=0; _size=0; _maxSize=0; }

        /**
         * Tighten the allocated memory to the size of the buffer.
         * @return the new pointer to the first element.
         * @version 1.0
         */
        T* pack()
            {
                if( _maxSize != _size )
                {
                    _data = static_cast< T* >( realloc( _data,
                                                        _size * sizeof( T )));
                    _maxSize = _size;
                }
                return _data;
            }

        /** Copy constructor, transfers ownership to new Buffer. @version 1.0 */
        Buffer( Buffer& from )
            {
                _data = from._data; _size = from._size; _maxSize =from._maxSize;
                from._data = 0; from._size = 0; from._maxSize = 0;
            }

        /** Assignment operator, copies data from Buffer. @version 1.0 */
        const Buffer& operator = ( Buffer& from )
            {
                replace( from._data, from._size );
                return *this;
            }

        /** Direct access to the element at the given index. @version 1.0 */
        T&       operator[]( const uint64_t position )
            { EQASSERT( _size > position ); return _data[ position ]; }

        /** Direct const access to an element. @version 1.0 */
        const T& operator[]( const uint64_t position ) const
            { EQASSERT( _size > position ); return _data[ position ]; }

        /** 
         * Ensure that the buffer contains at least newSize elements.
         *
         * Existing data is retained. The size is set.
         * @return the new pointer to the first element.
         * @version 1.0
         */
        T* resize( const uint64_t newSize )
            { 
                _size = newSize;
                if( newSize <= _maxSize )
                    return _data;

                // avoid excessive reallocs
                const uint64_t nElems = newSize + (newSize >> 3);
                const uint64_t nBytes = nElems * sizeof( T );
                _data = static_cast< T* >( realloc( _data, nBytes ));
                _maxSize = nElems;
                return _data;
            }

        /** 
         * Ensure that the buffer contains at least newSize elements.
         *
         * Existing data is retained. The size is increased, if necessary.
         * @version 1.0
         */
        void grow( const uint64_t newSize )
            { 
                if( newSize > _size )
                    resize( newSize );
            }

        /** 
         * Ensure that the buffer contains at least newSize elements.
         *
         * Existing data may be deleted.
         * @return the new pointer to the first element.
         * @version 1.0
         */
        T* reserve( const uint64_t newSize )
            {
                if( newSize <= _maxSize )
                    return _data;

                if( _data )
                    free( _data );
                
                _data = static_cast< T* >( malloc( newSize * sizeof( T )));
                _maxSize = newSize;
                return _data;
            }

        /** 
         * Set the buffer size and malloc enough memory.
         *
         * Existing data may be deleted.
         * @return the new pointer to the first element.
         * @version 1.0
         */
        T* reset( const uint64_t newSize )
            {
                reserve( newSize );
                setSize( newSize );
                return _data;
            }

        /** Append elements to the buffer, increasing the size. @version 1.0 */
        void append( const T* data, const uint64_t size )
            {
                EQASSERT( data );
                EQASSERT( size );

                const uint64_t oldSize = _size;
                resize( oldSize + size );
                memcpy( _data + oldSize, data, size * sizeof( T ));
            }

        /** Append one element to the buffer. @version 1.0 */
        void append( const T& element )
            {
                resize( _size + 1 );
                _data[ _size - 1 ] = element;
            }

        /** Replace the existing data with new data. @version 1.0 */
        void replace( const void* data, const uint64_t size )
            {
                EQASSERT( data );
                EQASSERT( size );

                reserve( size );
                memcpy( _data, data, size * sizeof( T ));
                _size = size;
            }

        /** Swap the buffer contents with another Buffer. @version 1.0 */
        void swap( Buffer& buffer )
            {
                T*             tmpData    = buffer._data;
                const uint64_t tmpSize    = buffer._size;
                const uint64_t tmpMaxSize = buffer._maxSize;

                buffer._data = _data;
                buffer._size = _size;
                buffer._maxSize = _maxSize;

                _data     = tmpData;
                _size     = tmpSize;
                _maxSize = tmpMaxSize;
            }

        /** @return a pointer to the data. @version 1.0 */
        T* getData() { return _data; }

        /** @return a const pointer to the data. @version 1.0 */
        const T* getData() const { return _data; }

        /**
         * Set the size of the buffer without changing its allocation.
         *
         * This method only modifies the size parameter. If the current
         * allocation of the buffer is too small, it asserts, returns false and
         * does not change the size.
         * @version 1.0
         */
        bool setSize( const uint64_t size )
            {
                EQASSERT( size <= _maxSize );
                if( size > _maxSize )
                    return false;

                _size = size;
                return true;
            }
                    
        /** @return the current size. @version 1.0 */
        uint64_t getSize() const { return _size; }
        
        /** @return true if the buffer is empty, false if not. @version 1.0 */
        bool isEmpty() const { return (_size==0); }
        
        /** @return the maximum size of the buffer. @version 1.0 */
        uint64_t getMaxSize() const { return _maxSize; }

    private:
        /** A pointer to the data. */
        T* _data;

        /** The number of valid items in _data. */
        uint64_t _size;

        /** The allocation _size of the buffer. */
        uint64_t _maxSize;
    };

    typedef Buffer< uint8_t > Bufferb;
}

}
#endif //COBASE_BUFFER_H
