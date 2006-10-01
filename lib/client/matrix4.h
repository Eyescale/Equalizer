
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef CLIENT_MATRIX4_H
#define CLIENT_MATRIX4_H

#include <eq/vmmlib/Matrix4.h>
#include <eq/net/object.h>

namespace eq
{
    template< typename T >
    class Matrix4 : public vmml::Matrix4<T>, public eqNet::Object
    {
    public:
        Matrix4();
        Matrix4( const void* data, uint64_t dataSize );

        Matrix4& operator= ( const Matrix4& mm )
            { vmml::Matrix4<T>::operator= (mm); return *this; }
    };

    typedef Matrix4<float> Matrix4f;

    template< class T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Matrix4<T>& matrix )
    {
        os << eqBase::disableFlush << eqBase::disableHeader << "Matrix " 
           << std::endl;
        os << "  " << matrix.m[0][0] << ", " << matrix.m[0][1] << ", " 
           << matrix.m[0][2] << ", " << matrix.m[0][3] << std::endl;
        os << "  " << matrix.m[1][0] << ", " << matrix.m[1][1] << ", " 
           << matrix.m[1][2] << ", " << matrix.m[1][3] << std::endl;
        os << "  " << matrix.m[2][0] << ", " << matrix.m[2][1] << ", " 
           << matrix.m[2][2] << ", " << matrix.m[2][3] << std::endl;
        os << "  " << matrix.m[3][0] << ", " << matrix.m[3][1] << ", " 
           << matrix.m[3][2] << ", " << matrix.m[3][3] << std::endl;
        os << eqBase::enableHeader << eqBase::enableFlush;
        return os;
    }
}

#endif //CLIENT_MATRIX4_H
