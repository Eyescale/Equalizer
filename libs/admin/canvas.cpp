
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "canvas.h"

#include "config.h"
#include "layout.h"
#include "nodeFactory.h"
#include "segment.h"
#include "server.h"

namespace eq
{
namespace admin
{
typedef fabric::Canvas< Config, Canvas, Segment, Layout > Super;

Canvas::Canvas( Config* parent )
        : Super( parent )
{
}

Canvas::~Canvas()
{
}

ServerPtr Canvas::getServer() 
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

}
}

#include "../fabric/canvas.ipp"
template class eq::fabric::Canvas< eq::admin::Config, eq::admin::Canvas,
                                   eq::admin::Segment, eq::admin::Layout >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::admin::Super& );
/** @endcond */

