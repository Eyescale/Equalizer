
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
#include "global.h"
#include "nameFinder.h"
#include "nodeFactory.h"
#include "view.h"

namespace eq
{
typedef fabric::Layout< Config, Layout, View > Super;

Layout::Layout( Config* parent )
        : Super( parent )
{
}

Layout::~Layout()
{
    EQASSERT( getViews().empty( ));
}

View* Layout::createView()
{
    NodeFactory* nodeFactory = Global::getNodeFactory();
    return nodeFactory->createView( this );
}

void Layout::releaseView( View* view )
{
    NodeFactory* nodeFactory = Global::getNodeFactory();
    nodeFactory->releaseView( view );
}

void Layout::_unmap()
{
    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( !isMaster( ));

    const ViewVector& views = getViews();
    while( !views.empty( ))
    {
        View* view = views.back();
        EQASSERT( view->getID() != EQ_ID_INVALID );

        config->unmapObject( view );
        _removeView( view );
        releaseView( view );
    }

    config->unmapObject( this );
}

}

#include "../fabric/layout.cpp"
template class eq::fabric::Layout< eq::Config, eq::Layout, eq::View >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                const eq::fabric::Layout< eq::Config, eq::Layout, eq::View >& );
/** @endcond */
