
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef CLIENT_MATRIX4_H
#define CLIENT_MATRIX4_H

#include <eq/net/object.h>
#include <eq/vmmlib/Matrix4.h>

namespace eq
{
    template< typename T >
    class Matrix4 : public vmml::Matrix4<T>, public eqNet::Object
    {
    public:
        Matrix4();
        Matrix4( const vmml::Matrix4<T>& matrix );
        virtual ~Matrix4(){}

        Matrix4& operator= ( const Matrix4<T>& matrix )
            { vmml::Matrix4<T>::operator= (matrix); return *this; }
        Matrix4& operator= ( const vmml::Matrix4<T>& matrix )
            { vmml::Matrix4<T>::operator= (matrix); return *this; }

    protected:

        virtual bool isStatic() const { return false; }
    };

    typedef Matrix4<float> Matrix4f;
    typedef Matrix4<double> Matrix4d;

    // Implementation

    template< class T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Matrix4<T>& matrix )
    {
        os << eqBase::disableFlush << eqBase::disableHeader << eqBase::indent
           << static_cast< const vmml::Matrix4<T>& >( matrix )
           << eqBase::exdent << eqBase::enableHeader << eqBase::enableFlush;
        return os;
    }

    template< class T >
    Matrix4<T>::Matrix4() 
    {
        vmml::Matrix4<T>::operator= ( vmml::Matrix4<T>::IDENTITY );
        setInstanceData( &(this->ml), 16 * sizeof( T ));
    }

    template< class T >
    Matrix4<T>::Matrix4( const vmml::Matrix4<T>& matrix )
            : vmml::Matrix4<T>( matrix )
    {
        setInstanceData( &(this->ml), 16 * sizeof( T ));
    }
}

#endif //CLIENT_MATRIX4_H
