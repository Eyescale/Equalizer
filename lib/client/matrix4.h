
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   All rights reserved. */

#ifndef CLIENT_MATRIX4_H
#define CLIENT_MATRIX4_H

#include <math.h>

namespace eq
{
    template< typename T >
    class Matrix4 /*: public eqNet::Object*/
    {
    public:
        void makeIdentity();
        void setTranslation( const T x, const T y, const T z );

        void rotateX( const T angle );
        void rotateY( const T angle );
        void rotateZ( const T angle );

        const T* getMatrix() const { return _m; } 

    protected:
        /*
          const void* getInstanceData( uint64_t* size )
          {
          return pack( size );
          }

          const void* pack( uint64_t* size )
          {
          *size = 16*sizeof( T );
          return _data;
          }

          void unpack( const void* data, const uint64_t size )
          {
          EQASSERT( size == 16*sizeof( T ));
          memcpy( _data, data, 16*sizeof( T ));
          }
        */
    private:
        T _m[16];
    };

#   include "matrix4.ipp" // template implementation

    typedef Matrix4<float> Matrix4f;
}

#endif //CLIENT_MATRIX4_H
