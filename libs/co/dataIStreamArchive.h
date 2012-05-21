
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@googlemail.com>
 *               2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef CO_DATAISTREAMARCHIVE_H
#define CO_DATAISTREAMARCHIVE_H

#include <co/defines.h>
#ifdef CO_USE_BOOST_SERIALIZATION

#include <co/api.h>
#include <co/dataIStream.h>

#pragma warning( push )
#pragma warning( disable: 4800 )
#include <boost/archive/basic_binary_iarchive.hpp>
#pragma warning( pop )
#include <boost/archive/detail/register_archive.hpp>
#include <boost/archive/shared_ptr_helper.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>

namespace co
{
    /** A boost.serialization archive for a co::DataIStream. */
    class DataIStreamArchive
        : public boost::archive::basic_binary_iarchive< DataIStreamArchive >
        , public boost::archive::detail::shared_ptr_helper
    {
        typedef boost::archive::basic_binary_iarchive<DataIStreamArchive> Super;

    public:
        /** Construct a new deserialization archive. */
        CO_API DataIStreamArchive( DataIStream& stream );

        /** @internal archives are expected to support this function */
        CO_API void load_binary( void* data, std::size_t size );

        /** @internal enable serialization optimization for arrays. */
        struct use_array_optimization
        {
            template< class T >
            struct apply
                : public boost::serialization::is_bitwise_serializable< T > {};
        };

        /** @internal use optimized load for arrays. */
        template< class ValueType >
        void load_array( boost::serialization::array< ValueType >& a,
                         unsigned int )
        {
            load_binary( a.address(), a.count() * sizeof( ValueType ));
        }

    private:
        friend class boost::archive::load_access;

        template<class T>
        void load( T& t )
        {
            _stream >> t;
        }

        DataIStream& _stream;
    };
}

// contains load_override impl for class_name_type
#include <boost/archive/impl/basic_binary_iarchive.ipp>

BOOST_SERIALIZATION_REGISTER_ARCHIVE(co::DataIStreamArchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(co::DataIStreamArchive)

#endif //CO_USE_BOOST_SERIALIZATION
#endif //CO_DATAISTREAMARCHIVE_H
