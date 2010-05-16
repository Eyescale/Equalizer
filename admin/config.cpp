
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

#include "config.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "layout.h"
#include "node.h"
#include "nodeFactory.h"
#include "observer.h"
#include "server.h"
#include "view.h"

#include "configCommitVisitor.h"

namespace eq
{
namespace admin
{
typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node, 
                        ConfigVisitor > Super;

Config::Config( ServerPtr parent )
        : Super( parent )
{}

Config::~Config()
{}

uint32_t Config::commit()
{
    const uint32_t result = Super::commit();
    ConfigCommitVisitor visitor;
    accept( visitor );
    return result;
}

}
}

#include "../lib/fabric/config.ipp"
template class eq::fabric::Config< eq::admin::Server, eq::admin::Config,
                                   eq::admin::Observer, eq::admin::Layout,
                                   eq::admin::Canvas, eq::admin::Node,
                                   eq::admin::ConfigVisitor >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::admin::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template void eq::admin::Super::find< type >( const uint32_t, type** );

FIND_ID_TEMPLATE1( eq::admin::Observer );
FIND_ID_TEMPLATE1( eq::admin::Layout );

#define FIND_ID_TEMPLATE2( type )                                       \
    template type* eq::admin::Super::find< type >( const uint32_t );

FIND_ID_TEMPLATE2( eq::admin::Observer );
FIND_ID_TEMPLATE2( eq::admin::Layout );
FIND_ID_TEMPLATE2( eq::admin::View );

#define FIND_NAME_TEMPLATE1( type )\
    template void eq::admin::Super::find< type >(const std::string&,   \
                                          const type** ) const;
FIND_NAME_TEMPLATE1( eq::admin::Observer );
FIND_NAME_TEMPLATE1( eq::admin::Layout );


#define CONST_FIND_NAME_TEMPLATE2( type )                               \
    template const type* eq::admin::Super::find< type >( const std::string& ) const;

CONST_FIND_NAME_TEMPLATE2( eq::admin::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::admin::View );


