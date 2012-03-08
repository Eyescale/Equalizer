
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

#ifdef CO_USE_BOOST

namespace co
{

DataIStreamArchive::DataIStreamArchive( DataIStream& stream )
: _stream( stream )
{
}

void DataIStreamArchive::load_binary( void* data, std::size_t size )
{
    _stream.read( data, size );
}

}

#endif //CO_USE_BOOST
