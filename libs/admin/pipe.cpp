
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

#include "pipe.h"

#include "client.h"
#include "config.h"
#include "node.h"
#include "nodeFactory.h"
#include "server.h"
#include "window.h"

namespace eq
{
namespace admin
{
typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;

Pipe::Pipe( Node* parent )
        : Super( parent )
{}

Pipe::~Pipe()
{}

Config* Pipe::getConfig()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}

const Config* Pipe::getConfig() const
{
    const Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}


ServerPtr Pipe::getServer()
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

ClientPtr Pipe::getClient()
{
    ServerPtr server = getServer();
    EQASSERT( server.isValid( ));

    if( !server )
        return 0;
    return server->getClient();
}

co::CommandQueue* Pipe::getMainThreadQueue()
{
    return getServer()->getMainThreadQueue();
}


}
}

#include "../fabric/pipe.ipp"
template class eq::fabric::Pipe< eq::admin::Node, eq::admin::Pipe,
                                 eq::admin::Window, eq::admin::PipeVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::admin::Super& );
/** @endcond */
