
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

#include "view.h"

#include "config.h"
#include "layout.h"
#include "observer.h"
#include "server.h"

namespace eq
{
namespace admin
{
typedef fabric::View< Layout, View, Observer > Super;

View::View( Layout* parent )
        : Super( parent )
{}

View::~View()
{}

Config* View::getConfig()
{
    Layout* layout = getLayout();
    EQASSERT( layout );
    return layout ? layout->getConfig() : 0;
}

const Config* View::getConfig() const
{
    const Layout* layout = getLayout();
    EQASSERT( layout );
    return layout ? layout->getConfig() : 0;
}

ServerPtr View::getServer() 
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

}
}

#include "../fabric/view.ipp"
template class eq::fabric::View< eq::admin::Layout, eq::admin::View,
                                 eq::admin::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::admin::Super& );
/** @endcond */
