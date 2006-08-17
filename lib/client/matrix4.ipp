/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   All rights reserved. */

template< typename T >
void Matrix4<T>::makeIdentity()
{
    for( int i=0; i<16; i++ )
    {
        if( i%5 == 0 )
            _m[i] = 1;
        else
            _m[i] = 0;
    }
}

template< typename T >
void Matrix4<T>::setTranslation( const T x, const T y, const T z )
{
    _m[3] = x;
    _m[7] = y;
    _m[11] = z;
}

template< typename T >
void Matrix4<T>::scale( const T scale[3] )
{
    _m[0] *= scale[0];
    _m[4] *= scale[0];
    _m[8] *= scale[0];
    _m[12] *= scale[0];
    _m[1] *= scale[1];
    _m[5] *= scale[1];
    _m[9] *= scale[1];
    _m[13] *= scale[1];
    _m[2] *= scale[2];
    _m[6] *= scale[2];
    _m[10] *= scale[2];
    _m[14] *= scale[2];
}

template< typename T >
void Matrix4<T>::rotateX( const T angle )
{
    //matrix multiplication: _m = _m * rotation x axis
    const T sinus = sin(angle);
    const T cosin = cos(angle);
    for( int i=0; i<16; i+=4 )
    {
        const T temp = _m[i];
        _m[i] = _m[i] * cosin - _m[i+2] * sinus;
        _m[i+2] = temp * sinus + _m[i+2] * cosin;
    }
}

template< typename T >
void Matrix4<T>::rotateY( const T angle )
{
    //matrix multiplication: _m = _m * rotation y axis
    const T sinus = sin(angle);
    const T cosin = cos(angle);
    for( int i=0; i<16; i+=4 )
    {
        const T temp = _m[i+1];
        _m[i+1] = _m[i+1] * cosin + _m[i+2] * sinus;
        _m[i+2] = temp * -sinus + _m[i+2] * cosin;
    }
}

template< typename T >
void Matrix4<T>::rotateZ( const T angle )
{
    //matrix multiplication: _m = _m * rotation z axis
    const T sinus = sin(angle);
    const T cosin = cos(angle);
    for( int i=0; i<16; i+=4 )
    {
        const T temp = _m[i];
        _m[i] = _m[i] * cosin + _m[i+1] * sinus;
        _m[i+1] = temp * -sinus + _m[i+1] * cosin;
    }
}

template< typename T >
void Matrix4<T>::operator *= ( const Matrix4<T>& rhs )
{
    T result[16] = {0};
    for( int i=0;i<4;i++ )
    {
        for (int k=0;k<4;k++ )
        {
            for (int n=0;n<4;n++ )
            {
                //The rhs matrix is the left matrix in the multiplication
                result[4*i+k] += rhs._m[4*i+n] * _m[n*4+k];
            }
        }
    }
    memcpy( _m, result, 16*sizeof( T ));
}
