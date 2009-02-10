
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include "channel.h"
#include "config.h"
#include "layout.h"
#include "paths.h"

using namespace eq::base;

namespace eq
{
namespace server
{

View::View()
        : _layout( 0 )
{
}

View::View( const View& from )
        : eq::View( from )
        , _layout( 0 )
{
    for( ChannelVector::const_iterator i = from._channels.begin();
         i != from._channels.end(); ++i )
    {
        const Channel* oldChannel = *i;
        const ChannelPath path( oldChannel->getPath( ));

        Channel* newChannel = getConfig()->getChannel( path );
        EQASSERT( newChannel );

        addChannel( newChannel );
    }
}

View::~View()
{
    if( _layout )
        _layout->removeView( this );
    _layout = 0;
}

void View::setViewport( const Viewport& viewport )
{
    _viewport = viewport;
    setDirty( DIRTY_VIEWPORT );
}

Config* View::getConfig()
{
    EQASSERT( _layout );
    return _layout ? _layout->getConfig() : 0;
}


const Config* View::getConfig() const
{
    EQASSERT( _layout );
    return _layout ? _layout->getConfig() : 0;
}

void View::addChannel( Channel* channel )
{
    _channels.push_back( channel );
    channel->setView( this );
}

bool View::removeChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _channels.begin(), 
                                      _channels.end(), channel );

    if( i == _channels.end( ))
        return false;

    channel->setView( 0 );
    _channels.erase( i );
    return true;
}

std::ostream& operator << ( std::ostream& os, const View* view )
{
    if( !view )
        return os;
    
    os << disableFlush << disableHeader << "view" << std::endl;
    os << "{" << std::endl << indent;
    
    const std::string& name = view->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const eq::Viewport& vp  = view->getViewport();
    if( vp.isValid( ) && vp != eq::Viewport::FULL )
        os << "viewport " << vp << std::endl;

    switch( view->getCurrentType( ))
    {
        case eq::View::TYPE_WALL:
            os << view->getWall() << std::endl;
            break;
        case eq::View::TYPE_PROJECTION:
            os << view->getProjection() << std::endl;
            break;
        default: 
            break;
    }

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
