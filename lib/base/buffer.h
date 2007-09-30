
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BUFFER_H
#define EQBASE_BUFFER_H

#include <eq/base/nonCopyable.h> // base class

namespace eqBase
{
    /**
     * A simple memory buffer with some helper functions.
     */
    class Buffer  : public eqBase::NonCopyable
    {
    public:
        Buffer() : data(0), size(0), _maxSize(0) {}
        ~Buffer() { if( data ) free( data ); data=0; }

        /** Ensure that the data can contain at least size bytes */
        void resize( const uint64_t size )
            { 
                this->size = size;
                if( size <= _maxSize )
                    return;
                
                if( data )
                    data = static_cast< char* >( realloc( data, size ));
                else
                    data = static_cast< char* >( malloc( size ));
                
                _maxSize = size;
            }

        /** Append size bytes to the buffer, increasing its size. */
        void append( const void* data, const uint64_t size )
            {
                const uint64_t oldSize = this->size;
                resize( oldSize + size );
                memcpy( this->data + oldSize, data, size );
            }

        /** Swap the buffer contents */
        void swap( Buffer& buffer )
            {
                char*          tmpData = buffer.data;
                const uint64_t tmpSize    = buffer.size;
                const uint64_t tmpMaxSize = buffer._maxSize;

                buffer.data = data;
                buffer.size = size;
                buffer._maxSize = _maxSize;

                data     = tmpData;
                size     = tmpSize;
                _maxSize = tmpMaxSize;
            }

        /** A pointer to the data, char* for easier pointer math */
        char*    data;
        /** The number of valid bytes in data. */
        uint64_t size;

    private:
        /** The allocation size of the buffer. */
        uint64_t _maxSize;
    };
}

#endif //EQBASE_BUFFER_H
