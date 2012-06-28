
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
 *
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


template< class ValueType >
void DataIStreamArchive::load_array(
                                    boost::serialization::array< ValueType >& a,
                                    unsigned int )
{
    load_binary( a.address(), a.count() * sizeof( ValueType ));
}


template<class C, class T, class A>
void DataIStreamArchive::load( std::basic_string<C, T, A>& s )
{
    // implementation only valid for narrow string
    BOOST_STATIC_ASSERT( sizeof(C) == sizeof(char));
    _stream >> s;
}

template< typename T >
typename boost::enable_if< boost::is_integral<T> >::type
DataIStreamArchive::load( T& t )
{
    // get the number of bytes in the stream
    if( signed char size = _loadSignedChar( ))
    {
        // check for negative value in unsigned type
        if( size < 0 && boost::is_unsigned<T>::value )
            throw DataStreamArchiveException();

        // check that our type T is large enough
        else if( (unsigned)abs(size) > sizeof(T))
            throw DataStreamArchiveException( size );

        // reconstruct the value
        T temp = size < 0 ? -1 : 0;
        load_binary( &temp, abs(size));

        // load the value from little endian - is is then converted
        // to the target type T and fits it because size <= sizeof(T)
        t = boost::detail::load_little_endian<T, sizeof(T)>( &temp );
    }
    else
        // zero optimization
        t = 0;
}

template< typename T >
typename boost::enable_if<boost::is_floating_point<T> >::type
DataIStreamArchive::load( T& t )
{
    namespace fp = boost::spirit::math;

    typedef typename fp::detail::fp_traits<T>::type traits;

    // if you end here there are three possibilities:
    // 1. you're serializing a long double which is not portable
    // 2. you're serializing a double but have no 64 bit integer
    // 3. your machine is using an unknown floating point format
    // after reading the note above you still might decide to
    // deactivate this static assert and try if it works out.
    typename traits::bits bits;
    BOOST_STATIC_ASSERT( sizeof(bits) == sizeof(T));
    BOOST_STATIC_ASSERT( std::numeric_limits<T>::is_iec559 );

    load( bits );
    traits::set_bits( t, bits );

    // if the no_infnan flag is set we must throw here
    if( get_flags() & no_infnan && !fp::isfinite( t ))
        throw DataStreamArchiveException( t );

    // if you end here your floating point type does not support
    // denormalized numbers. this might be the case even though
    // your type conforms to IEC 559 (and thus to IEEE 754)
    if( std::numeric_limits<T>::has_denorm == std::denorm_absent &&
        fp::fpclassify(t) == (int)FP_SUBNORMAL ) // GCC4
    {
        throw DataStreamArchiveException( t );
    }
}
