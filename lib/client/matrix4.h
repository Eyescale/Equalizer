
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
        Matrix4( const vmml::Matrix4<T>& matrix );
        Matrix4( const void* data, uint64_t dataSize );

        Matrix4& operator= ( const Matrix4<T>& matrix )
            { vmml::Matrix4<T>::operator= (matrix); return *this; }
        Matrix4& operator= ( const vmml::Matrix4<T>& matrix )
            { vmml::Matrix4<T>::operator= (matrix); return *this; }
    };

    typedef Matrix4<float> Matrix4f;

    template< class T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Matrix4<T>& matrix )
    {
        os << eqBase::disableFlush << eqBase::disableHeader << eqBase::indent
           << (const vmml::Matrix4<T>&) matrix;
        os << eqBase::exdent << eqBase::enableHeader << eqBase::enableFlush;
        return os;
    }
}

#endif //CLIENT_MATRIX4_H
