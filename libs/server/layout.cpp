
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

#include "layout.h"

#include "config.h"
#include "view.h"

#include <eq/fabric/paths.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{
typedef fabric::Layout< Config, Layout, View > Super;

Layout::Layout( Config* parent )
        : Super( parent )
        , _state( STATE_ACTIVE )
{
}

Layout::~Layout()
{
}

ServerPtr Layout::getServer() 
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

void Layout::postDelete()
{
    _state = STATE_DELETE;
    getConfig()->postNeedsFinish();
}

void Layout::trigger( const Canvas* canvas, const bool active )
{
    EQASSERT( canvas );
    getConfig()->postNeedsFinish();

    const Views& views = getViews();
    for( Views::const_iterator i = views.begin(); i != views.end(); ++i )
    {
        View* view = *i;
        view->trigger( canvas, active );
    }
}

}
}

#include "nodeFactory.h"
#include "../fabric/layout.ipp"

template class eq::fabric::Layout< eq::server::Config, eq::server::Layout,
                                   eq::server::View >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::fabric::Layout<
                                                 eq::server::Config,
                                                 eq::server::Layout,
                                                 eq::server::View >& );
/** @endcond */
