
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "event.h"
#include "pipe.h"
#include "layout.h"
#include "observer.h"
#include "server.h"

#include <co/command.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
typedef fabric::View< Layout, View, Observer > Super;

View::View( Layout* parent )
        : Super( parent )
        , _pipe( 0 )
{
}

View::~View()
{
}

void View::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );
    if( _baseFrustum.getCurrentType() == TYPE_NONE &&
        ( dirtyBits & DIRTY_FRUSTUM ))
    {
        _baseFrustum = *this; // save baseline data for resizing
    }
}

void View::detach()
{
    // if pipe is not running, detach comes from _flushViews in state stopping
    //  Don't send packet to stopping pipe (see issue #11)
    if( _pipe && _pipe->isRunning( ))
    {
        co::LocalNodePtr localNode = getLocalNode();

        co::BufferPtr buffer = localNode->allocCommand( 8 );
        co::ObjectOCommand command( co::Connections(),
                                    co::COMMANDTYPE_CO_OBJECT,
                                    fabric::CMD_PIPE_DETACH_VIEW,
                                    _pipe->getID(), EQ_INSTANCE_ALL );
        command << getID();
        buffer->swap( command.getBuffer( ));
        _pipe->dispatchCommand( buffer );
    }
    Super::detach();
}

Config* View::getConfig()
{
    Layout* layout = getLayout();
    if( layout )
        return layout->getConfig();

    if( _pipe )
        return _pipe->getConfig();

    LBUNREACHABLE;
    return 0;
}

const Config* View::getConfig() const
{
    const Layout* layout = getLayout();
    if( layout )
        return layout->getConfig();

    if( _pipe )
        return _pipe->getConfig();

    LBUNREACHABLE;
    return 0;
}

ServerPtr View::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

bool View::handleEvent( const Event& event )
{
    switch( event.type )
    {
        case Event::VIEW_RESIZE:
        {
            const ResizeEvent& resize = event.resize;
            if( resize.dw == 0.f || resize.dh == 0.f )
                return true;

            switch( getCurrentType( ))
            {
                case TYPE_WALL:
                {
                    const float ratio( resize.dw / resize.dh );
                    Wall wall( _baseFrustum.getWall( ));

                    wall.resizeHorizontal( ratio );
                    setWall( wall );
                    break;
                }

                case View::TYPE_PROJECTION:
                {
                    const float ratio( resize.dw / resize.dh );
                    eq::Projection projection( _baseFrustum.getProjection( ));

                    projection.resizeHorizontal( ratio );
                    setProjection( projection );
                    break;
                }

                case eq::View::TYPE_NONE:
                    break;
                default:
                    LBUNIMPLEMENTED;
                    break;
            }

            return true;
        }
    }

    return false;
}

void View::freezeLoadBalancing( const bool onOff )
{
    send( getServer(), fabric::CMD_VIEW_FREEZE_LOAD_BALANCING ) << onOff;
}

}

#include "../fabric/view.ipp"
template class eq::fabric::View< eq::Layout, eq::View, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                             const eq::Super& );
/** @endcond */
