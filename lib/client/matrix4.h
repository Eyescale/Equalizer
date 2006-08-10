
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   All rights reserved. */

#ifndef CLIENT_MATRIX4_H
#define CLIENT_MATRIX4_H

#include <math.h>
#include <eq/net/object.h>
#include <eq/client/packets.h>

namespace eq
{
    template< typename T >
    class Matrix4 : public eqNet::Object
    {
    public:
        Matrix4();
        Matrix4( const void* data, uint64_t dataSize );

        void makeIdentity();
        void setTranslation( const T x, const T y, const T z );

        void rotateX( const T angle );
        void rotateY( const T angle );
        void rotateZ( const T angle );

        const T* getMatrix() const { return _m; } 

        Matrix4& operator = ( const Matrix4& matrix )
            {
                memcpy( _m, matrix._m, 16*sizeof( T ) );
                return *this;
            }

    protected:
          const void* getInstanceData( uint64_t* size )
          {
              return pack( size );
          }

          const void* pack( uint64_t* size )
          {
              *size = 16*sizeof( T );
              return _m;
          }

          void unpack( const void* data, const uint64_t size )
          {
              EQASSERT( size == 16*sizeof( T ) );
              memcpy( _m, data, size );
          }

    private:
        T _m[16];
    };

#   include "matrix4.ipp" // template implementation

    typedef Matrix4<float> Matrix4f;

    template< class T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Matrix4<T>& matrix )
    {
        os << eqBase::disableFlush << eqBase::disableHeader << "Matrix " 
           << std::endl;
        T values[16];
        memcpy( values, matrix.getMatrix(), 16*sizeof( T ) );
        for( int i=0; i<16; i+=4 )
        {
            os << "  " << values[i] << "  " << values[i+1] << "  " << 
                      values[i+2] << "  " << values[i+3] << std::endl;
        }
        os << std::endl << eqBase::enableHeader << eqBase::enableFlush;
        return os;
    }
}

#endif //CLIENT_MATRIX4_H
