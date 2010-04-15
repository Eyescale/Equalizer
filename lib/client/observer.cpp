
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "observer.h"

#include "config.h"

namespace eq
{

Observer::Observer( Config* parent )
        : fabric::Observer< Config, Observer >( parent )
{
}

Observer::~Observer()
{
}

}

#include "../fabric/observer.cpp"
template class eq::fabric::Observer< eq::Config, eq::Observer >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                      const eq::fabric::Observer< eq::Config, eq::Observer >& );
/** @endcond */
