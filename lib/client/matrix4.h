
/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_MATRIX4_H
#define EQ_MATRIX4_H

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/net/object.h>
#include <vmmlib/matrix4.h>

namespace eq
{
    template< typename T >
    class Matrix4 : public vmml::Matrix4<T>, public net::Object
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
        virtual ChangeType getChangeType() const { return UNBUFFERED; }
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );
    };

    typedef Matrix4<float> Matrix4f;
    typedef Matrix4<double> Matrix4d;

    // Implementation

    template< class T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Matrix4<T>& matrix )
    {
        os << base::disableFlush << base::disableHeader << base::indent
           << static_cast< const vmml::Matrix4<T>& >( matrix )
           << base::exdent << base::enableHeader << base::enableFlush;
        return os;
    }

    template< class T >
    Matrix4<T>::Matrix4() 
    {
        vmml::Matrix4<T>::operator= ( vmml::Matrix4<T>::IDENTITY );
    }

    template< class T >
    Matrix4<T>::Matrix4( const vmml::Matrix4<T>& matrix )
            : vmml::Matrix4<T>( matrix )
    {}

    template< class T >
    void Matrix4<T>::getInstanceData( net::DataOStream& os )
    {
        os.writeOnce( &(this->ml), sizeof( this->ml )); 
    }

    template< class T >
    void Matrix4<T>::applyInstanceData( net::DataIStream& is )
    {
        EQASSERT( is.getRemainingBufferSize() == sizeof( this->ml )); 

        memcpy( &(this->ml), is.getRemainingBuffer(), sizeof( this->ml ));
        is.advanceBuffer( sizeof( this->ml ));

        EQASSERT( is.nRemainingBuffers() == 0 );
        EQASSERT( is.getRemainingBufferSize() == 0 );
    }
}

#endif //EQ_MATRIX4_H
