
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch>
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
#include "client.h"
#include "layout.h"
#include "node.h"
#include "nodeFactory.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

namespace eq
{
namespace admin
{
Config::Config( ServerPtr parent )
        : Super( parent )
{}

Config::~Config()
{}

co::CommandQueue* Config::getMainThreadQueue()
{
    return getClient()->getMainThreadQueue();
}

ClientPtr Config::getClient()
{
    return getServer()->getClient();
}

ConstClientPtr Config::getClient() const
{
    return getServer()->getClient();
}

uint128_t Config::commit( const uint32_t /*incarnation*/ )
{
    return Super::commit( CO_COMMIT_NEXT );
}

}
}

#include "../fabric/config.ipp"
#include "../fabric/view.ipp"
#include "../fabric/observer.ipp"
template class eq::fabric::Config< eq::admin::Server, eq::admin::Config,
                                   eq::admin::Observer, eq::admin::Layout,
                                   eq::admin::Canvas, eq::admin::Node,
                                   eq::admin::ConfigVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                              const eq::admin::Config::Super& );
/** @endcond */

#define FIND_ID_TEMPLATE1( type )                                       \
    template void eq::admin::Config::Super::find< type >( const uint128_t&, \
                                                          type** );

FIND_ID_TEMPLATE1( eq::admin::Canvas );
FIND_ID_TEMPLATE1( eq::admin::Channel );
FIND_ID_TEMPLATE1( eq::admin::Layout );
FIND_ID_TEMPLATE1( eq::admin::Node );
FIND_ID_TEMPLATE1( eq::admin::Observer );
FIND_ID_TEMPLATE1( eq::admin::Pipe );
FIND_ID_TEMPLATE1( eq::admin::Segment );
FIND_ID_TEMPLATE1( eq::admin::View );
FIND_ID_TEMPLATE1( eq::admin::Window );

#define FIND_ID_TEMPLATE2( type )                                       \
    template type* eq::admin::Config::Super::find< type >( const uint128_t& );

FIND_ID_TEMPLATE2( eq::admin::Canvas );
FIND_ID_TEMPLATE2( eq::admin::Channel );
FIND_ID_TEMPLATE2( eq::admin::Layout );
FIND_ID_TEMPLATE2( eq::admin::Node );
FIND_ID_TEMPLATE2( eq::admin::Observer );
FIND_ID_TEMPLATE2( eq::admin::Pipe );
FIND_ID_TEMPLATE2( eq::admin::Segment );
FIND_ID_TEMPLATE2( eq::admin::View );
FIND_ID_TEMPLATE2( eq::admin::Window );

#define FIND_NAME_TEMPLATE1( type )\
    template void eq::admin::Config::Super::find< type >(const std::string&, \
                                                         const type** ) const;
FIND_NAME_TEMPLATE1( eq::admin::Canvas );
FIND_NAME_TEMPLATE1( eq::admin::Channel );
FIND_NAME_TEMPLATE1( eq::admin::Layout );
FIND_NAME_TEMPLATE1( eq::admin::Node );
FIND_NAME_TEMPLATE1( eq::admin::Observer );
FIND_NAME_TEMPLATE1( eq::admin::Pipe );
FIND_NAME_TEMPLATE1( eq::admin::Segment );
FIND_NAME_TEMPLATE1( eq::admin::View );
FIND_NAME_TEMPLATE1( eq::admin::Window );

#define FIND_NAME_TEMPLATE2( type )                                     \
    template type*                                                      \
    eq::admin::Config::Super::find< type >( const std::string& );

FIND_NAME_TEMPLATE2( eq::admin::Canvas );
FIND_NAME_TEMPLATE2( eq::admin::Channel );
FIND_NAME_TEMPLATE2( eq::admin::Layout );
FIND_NAME_TEMPLATE2( eq::admin::Node );
FIND_NAME_TEMPLATE2( eq::admin::Observer );
FIND_NAME_TEMPLATE2( eq::admin::Pipe );
FIND_NAME_TEMPLATE2( eq::admin::Segment );
FIND_NAME_TEMPLATE2( eq::admin::View );
FIND_NAME_TEMPLATE2( eq::admin::Window );


#define CONST_FIND_NAME_TEMPLATE2( type )                               \
    template const type*                                                \
    eq::admin::Config::Super::find< type >( const std::string& ) const;

CONST_FIND_NAME_TEMPLATE2( eq::admin::Canvas );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Channel );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Layout );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Node );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Observer );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Pipe );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Segment );
CONST_FIND_NAME_TEMPLATE2( eq::admin::View );
CONST_FIND_NAME_TEMPLATE2( eq::admin::Window );
