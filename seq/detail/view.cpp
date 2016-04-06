
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "pipe.h"

#include <seq/application.h>
#include <seq/renderer.h>
#include <seq/viewData.h>
#include <eq/config.h>
#ifndef EQ_2_0_API
#  include <eq/configEvent.h>
#endif
#include <eq/eventICommand.h>

namespace seq
{
namespace detail
{

View::View( eq::Layout* parent )
        : eq::View( parent )
{}

View::~View()
{
}

Config* View::getConfig()
{
    return static_cast< Config* >( eq::View::getConfig( ));
}

Pipe* View::getPipe()
{
    return static_cast< Pipe* >( eq::View::getPipe( ));
}

ViewData* View::getViewData()
{
    return static_cast< ViewData* >( eq::View::getUserData( ));
}

const ViewData* View::getViewData() const
{
    return static_cast< const ViewData* >( eq::View::getUserData( ));
}

bool View::configInit()
{
    if( !eq::View::configInit( ))
        return false;

    if( !getPipe() ) // application view
        setUserData( getConfig()->getApplication()->createViewData( *this ));
    return true;
}

bool View::configExit()
{
    if( !getPipe() && getViewData( )) // application view
    {
        setUserData( 0 );
        getConfig()->getApplication()->destroyViewData( getViewData( ));
    }
    return eq::View::configExit();
}

void View::notifyAttach()
{
    eq::View::notifyAttach();
    Pipe* pipe = getPipe();

    if( pipe ) // render client view
        setUserData( pipe->getRenderer()->createViewData( *this ));
}

void View::notifyDetached()
{
    ViewData* data = getViewData();
    setUserData( 0 );

    if( data )
    {
        Pipe* pipe = getPipe();

        if( pipe ) // render client view
            pipe->getRenderer()->destroyViewData( data );
    }

    eq::View::notifyDetached();
}

bool View::updateData()
{
    if( !isActive( ))
        return false;

    ViewData* data = getViewData();
    LBASSERT( data );
    if( data )
        return data->update();
    return false;
}

#ifndef EQ_2_0_API
bool View::handleEvent( const eq::ConfigEvent* event )
{
    ViewData* data = getViewData();
    LBASSERT( data );
    if( !data )
        return false;
    if( isActive( ))
        return data->handleEvent( event );
    data->handleEvent( event );
    return false;
}
#endif

bool View::handleEvent( const eq::EventICommand& command )
{
    ViewData* data = getViewData();
    LBASSERT( data );
    if( !data )
        return false;
    if( isActive( ))
        return data->handleEvent( command );
    data->handleEvent( command );
    return false;
}
}
}
