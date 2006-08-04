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
