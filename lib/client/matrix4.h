
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef CLIENT_MATRIX4_H
#define CLIENT_MATRIX4_H

#include <eq/client/object.h>
#include <eq/vmmlib/Matrix4.h>

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

    // Specialized Constructors
    //   We need a new eq::Object type for each specialization. Below are the
    //   specialized constructors for all used template types.

    template<>
    inline Matrix4<float>::Matrix4() 
            : vmml::Matrix4f(),
              Object( eq::Object::TYPE_MATRIX4F )
    {
        vmml::Matrix4f::operator= ( vmml::Matrix4f::IDENTITY );
        setInstanceData( &ml, 16 * sizeof( float ));
    }

    template<>
    inline Matrix4<float>::Matrix4( const vmml::Matrix4f& matrix )
            : vmml::Matrix4f( matrix ),
              Object( eq::Object::TYPE_MATRIX4F )
    {
        setInstanceData( &ml, 16 * sizeof( float ));
    }

    template<>
    inline Matrix4<float>::Matrix4( const void* data, uint64_t dataSize )
            : vmml::Matrix4f( (float*)data ),
              Object( eq::Object::TYPE_MATRIX4F )
    {
        setInstanceData( &ml, 16 * sizeof( float ));
    }
}

#endif //CLIENT_MATRIX4_H
