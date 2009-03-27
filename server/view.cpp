
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "configVisitor.h"
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

View::View( const View& from, Config* config )
        : eq::View( from )
        , _layout( 0 )
{
    // _channels will be added by Segment copy ctor
}

View::~View()
{
    // Use copy - Channel::unsetOutput modifies vector
    ChannelVector channels = _channels;
    for( ChannelVector::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        Channel* channel = *i;
        channel->unsetOutput();
    }

    EQASSERT( _channels.empty( ));
    _channels.clear();

    if( _layout )
        _layout->removeView( this );
    _layout = 0;
}

namespace
{
class ViewUpdater : public ConfigVisitor
{
public:
    ViewUpdater( const ChannelVector& channels ) : _channels( channels ) {}
    virtual ~ViewUpdater() {}

    virtual VisitorResult visit( Compound* compound )
        {
            const Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;

            if( !compound->isDestination( ))
                return TRAVERSE_PRUNE; // only change destination compounds

            if( std::find( _channels.begin(), _channels.end(), channel ) !=
                _channels.end( )) // our destination channel
            {
                compound->updateFrustum();
            }

            return TRAVERSE_PRUNE;            
        }
private:
    const ChannelVector& _channels;
};
}

void View::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    eq::View::deserialize( is, dirtyBits );

    if( dirtyBits & ( DIRTY_WALL | DIRTY_PROJECTION ))
    {
        const ChannelVector& channels = getChannels();
        Config*              config   = getConfig();
        EQASSERT( config );

        ViewUpdater updater( channels );
        config->accept( updater );
    }
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
}

bool View::removeChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _channels.begin(), 
                                      _channels.end(), channel );

    EQASSERT( i != _channels.end( ));
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    return true;
}

ViewPath View::getPath() const
{
    EQASSERT( _layout );
    ViewPath path( _layout->getPath( ));
    
    const ViewVector&   views = _layout->getViews();
    ViewVector::const_iterator i = std::find( views.begin(),
                                              views.end(), this );
    EQASSERT( i != views.end( ));
    path.viewIndex = std::distance( views.begin(), i );
    return path;
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
