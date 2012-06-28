
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

#include "dataOStreamArchive.h"

#include <boost/archive/detail/archive_serializer_map.hpp>
#include <boost/archive/impl/archive_serializer_map.ipp>

namespace boost
{
namespace archive
{
template class CO_API detail::archive_serializer_map<co::DataOStreamArchive>;
}
}

namespace co
{

DataOStreamArchive::DataOStreamArchive( DataOStream& stream )
    : Super( 0 )
    , _stream( stream )
{
    // write our minimalistic header (magic byte plus version)
    // the boost archives write a string instead - by calling
    // boost::archive::basic_binary_oarchive<derived_t>::init()
    _saveSignedChar( magicByte );

    using namespace boost::archive;

#if BOOST_VERSION < 104400
    const version_type libraryVersion( BOOST_ARCHIVE_VERSION( ));
#else
    const library_version_type libraryVersion( BOOST_ARCHIVE_VERSION( ));
#endif
    operator<<( libraryVersion );
}

void DataOStreamArchive::save_binary( const void* data, std::size_t size )
{
    _stream.write( data, size );
}

void DataOStreamArchive::save( bool b )
{
    _saveSignedChar( b );
    if( b )
        _saveSignedChar( 'T' );
}

#if BOOST_VERSION >= 104400
void DataOStreamArchive::save(
                           const boost::archive::library_version_type& version )
{
    save((boost::uint_least16_t)(version));
}

void DataOStreamArchive::save( const boost::archive::class_id_type& class_id )
{
    save((boost::uint_least16_t)(class_id));
}

void DataOStreamArchive::save(
                       const boost::serialization::item_version_type& class_id )
{
    save((boost::uint_least32_t)(class_id));
}

void DataOStreamArchive::save(
                    const boost::serialization::collection_size_type& class_id )
{
    save((boost::uint_least32_t)(class_id));
}

void DataOStreamArchive::save( const boost::archive::object_id_type& object_id )
{
    save((boost::uint_least32_t)(object_id));
}

void DataOStreamArchive::save( const boost::archive::version_type& version )
{
    save((boost::uint_least32_t)(version));
}
#endif

void DataOStreamArchive::_saveSignedChar( const signed char& c )
{
    _stream << c;
}

}
