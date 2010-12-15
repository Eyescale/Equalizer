
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

#include "node.h"

#include "config.h"
#include "layout.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"

namespace eq
{
namespace admin
{
typedef fabric::Node< Config, Node, Pipe, NodeVisitor > Super;

Node::Node( Config* parent )
        : Super( parent )
{}

Node::~Node()
{}

ServerPtr Node::getServer()
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

}
}

#include "../fabric/node.ipp"
template class eq::fabric::Node< eq::admin::Config, eq::admin::Node,
                                 eq::admin::Pipe, eq::admin::NodeVisitor >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                const eq::admin::Super& );
/** @endcond */

