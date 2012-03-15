
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
}

void DataOStreamArchive::save_binary( void* data, std::size_t size )
{
    _stream.write( data, size );
}

}
