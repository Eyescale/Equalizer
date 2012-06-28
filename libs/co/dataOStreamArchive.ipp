
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


template< typename T >
void DataOStreamArchive::save_array( const boost::serialization::array< T >& a,
                                     unsigned int )
{
    save_binary( a.address(), a.count() * sizeof( T ));
}

template< class C, class T, class A >
void DataOStreamArchive::save( const std::basic_string< C, T, A >& s )
{
    // implementation only valid for narrow string
    BOOST_STATIC_ASSERT( sizeof(C) == sizeof(char));
    _stream << s;
}

template< typename T >
typename boost::enable_if< boost::is_integral<T> >::type
DataOStreamArchive::save( const T& t )
{
    if( T temp = t )
    {
        // examine the number of bytes
        // needed to represent the number
        signed char size = 0;
        do
        {
            temp >>= CHAR_BIT;
            ++size;
        }
        while( temp != 0 && temp != (T) -1 );

        // encode the sign bit into the size
        _saveSignedChar( t > 0 ? size : -size );
        BOOST_ASSERT( t > 0 || boost::is_signed<T>::value) ;

        // we choose to use little endian because this way we just
        // save the first size bytes to the stream and skip the rest
        boost::detail::store_little_endian<T, sizeof(T)>( &temp, t );
        save_binary( &temp, size );
    }
    else
        // zero optimization
        _saveSignedChar (0 );
}

template< typename T >
typename boost::enable_if< boost::is_floating_point<T> >::type
DataOStreamArchive::save( const T& t )
{
    namespace fp = boost::spirit::math;

    typedef typename fp::detail::fp_traits<T>::type traits;

    // if the no_infnan flag is set we must throw here
    if( get_flags() & no_infnan && !fp::isfinite( t ))
        throw DataStreamArchiveException( t );

    // if you end here there are three possibilities:
    // 1. you're serializing a long double which is not portable
    // 2. you're serializing a double but have no 64 bit integer
    // 3. your machine is using an unknown floating point format
    // after reading the note above you still might decide to
    // deactivate this static assert and try if it works out.
    typename traits::bits bits;
    BOOST_STATIC_ASSERT( sizeof(bits) == sizeof(T));
    BOOST_STATIC_ASSERT( std::numeric_limits<T>::is_iec559 );

    // examine value closely
    switch( fp::fpclassify( t ))
    {
    case FP_ZERO:
        bits = 0;
        break;
    case FP_NAN:
        bits = traits::exponent | traits::mantissa;
        break;
    case FP_INFINITE:
        bits = traits::exponent | (t<0) * traits::sign;
        break;
    case FP_SUBNORMAL:
        assert( std::numeric_limits<T>::has_denorm );
    case FP_NORMAL:
        traits::get_bits(t, bits);
        break;
    default:
        throw DataStreamArchiveException( t );
    }

    save( bits );
}
