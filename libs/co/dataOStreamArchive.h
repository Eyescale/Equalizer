
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

#ifndef CO_DATAOSTREAMARCHIVE_H
#define CO_DATAOSTREAMARCHIVE_H

#include <co/defines.h>
#ifdef CO_USE_BOOST_SERIALIZATION

#include <co/api.h>
#include <co/dataOStream.h>

#include <boost/archive/basic_binary_oarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>

namespace co
{
    /** A boost.serialization archive for a co::DataOStream. */
    class DataOStreamArchive
        : public boost::archive::basic_binary_oarchive< DataOStreamArchive >
    {
        typedef boost::archive::basic_binary_oarchive<DataOStreamArchive> Super;

    public:
        /** Construct a new serialization archive. */        
        CO_API DataOStreamArchive( DataOStream& stream );

        /** @internal archives are expected to support this function. */
        CO_API void save_binary( void* data, std::size_t size );

    private:
        friend class boost::archive::save_access;

        template<class T>
        void save( T& t )
        {
            _stream << t;
        }

        DataOStream& _stream;
    };
}

BOOST_SERIALIZATION_REGISTER_ARCHIVE(co::DataOStreamArchive)

#endif //CO_USE_BOOST_SERIALIZATION
#endif //CO_DATAOSTREAMARCHIVE_H
