
#ifndef MY_TESTING__H
#define MY_TESTING__H

#include <iostream>
#include <cstring> // size_t

// include assert in both debug and release modes
#ifdef NDEBUG
#  define DEF_NDEBUG
#  undef NDEBUG
#endif

#include <assert.h>

#ifdef DEF_NDEBUG
#  define NDEBUG
#endif

#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include <math.h> // fabs

namespace testing
{

template<typename T>
void testArrays( std::vector<T> a1, std::vector<T> a2 );

template<typename T>
void testArrays( const T* a1, const T* a2, size_t size );













//// IMPLEMENTATIONS

template<typename T>
void testArrays( std::vector<T> a1, std::vector<T> a2 )
{
    assert( a1.size() == a2.size() );

    for( size_t i = 0; i < a1.size(); ++i )
    {
        if( fabs( a1[i] - a2[i] ) > 0.00001 )
            std::cout << "i=" << i << " a1[i]=" << a1[i] << " a2[i]=" << a2[i] << std::endl;
        assert( fabs( a1[i] - a2[i] ) <= 0.00001 );
    }
}

template<>
void testArrays( std::vector<unsigned char> a1, std::vector<unsigned char> a2 )
{
    assert( a1.size() == a2.size() );

    for( size_t i = 0; i < a1.size(); ++i )
    {
        if( abs( int(a1[i]) - a2[i] ) > 0 )
            std::cout << "i=" << i << " a1[i]=" << (int)a1[i] << " a2[i]=" << (int)a2[i] << std::endl;
        assert( abs( int(a1[i]) - a2[i] ) <= 0 );
    }
}

template<typename T>
void testArrays( const T* a1, const T* a2, size_t size )
{
    for( size_t i = 0; i < size; ++i )
    {
        if( fabs( a1[i] - a2[i] ) > 0.00001 )
            std::cout << "i=" << i << " a1[i]=" << a1[i] << " a2[i]=" << a2[i] << std::endl;
        assert( fabs( a1[i] - a2[i] ) <= 0.00001 );
    }
}

template<>
void testArrays( const unsigned char* a1, const unsigned char* a2, size_t size )
{
    for( size_t i = 0; i < size; ++i )
    {
        if( abs( int(a1[i]) - a2[i] ) > 0 )
            std::cout << "i=" << i << " a1[i]=" << (int)a1[i] << " a2[i]=" << (int)a2[i] << std::endl;
        assert( abs( int(a1[i]) - a2[i] ) <= 0 );
    }
}

}

#endif // MY_TESTING__H

