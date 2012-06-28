
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

#include "dataIStreamArchive.h"

#include <boost/archive/detail/archive_serializer_map.hpp>
#include <boost/archive/impl/archive_serializer_map.ipp>

namespace boost
{
namespace archive
{
template class CO_API detail::archive_serializer_map<co::DataIStreamArchive>;
}
}

namespace co
{

DataIStreamArchive::DataIStreamArchive( DataIStream& stream )
    : Super( 0 )
    , _stream( stream )
{
    using namespace boost::archive;

    if( _loadSignedChar() != magicByte )
        throw archive_exception( archive_exception::invalid_signature );
    else
    {
#if BOOST_VERSION < 104400
        version_type libraryVersion;
#else
        library_version_type libraryVersion;
#endif
        operator>>( libraryVersion );

        if( libraryVersion > BOOST_ARCHIVE_VERSION( ))
            throw archive_exception( archive_exception::unsupported_version );
        else
            set_library_version( libraryVersion );
    }
}

void DataIStreamArchive::load_binary( void* data, std::size_t size )
{
    _stream.read( data, size );
}

void DataIStreamArchive::load( bool& b )
{
    switch( signed char c = _loadSignedChar( ))
    {
    case 0:
        b = false;
        break;
    case 1:
        b = _loadSignedChar();
        break;
    default:
        throw DataStreamArchiveException( c );
    }
}

#if BOOST_VERSION >= 104400
void DataIStreamArchive::load( boost::archive::library_version_type& version )
{
    load((boost::uint_least16_t&)(version));
}

void DataIStreamArchive::load( boost::archive::class_id_type& class_id )
{
    load((boost::uint_least16_t&)(class_id));
}

void DataIStreamArchive::load(
                              boost::serialization::item_version_type& version )
{
    load((boost::uint_least32_t&)(version));
}

void DataIStreamArchive::load(
                           boost::serialization::collection_size_type& version )
{
    load((boost::uint_least32_t&)(version));
}

void DataIStreamArchive::load( boost::archive::object_id_type& object_id )
{
    load((boost::uint_least32_t&)(object_id));
}

void DataIStreamArchive::load( boost::archive::version_type& version )
{
    load((boost::uint_least32_t&)(version));
}
#endif

signed char DataIStreamArchive::_loadSignedChar()
{
    signed char c;
    _stream >> c;
    return c;
}

}
