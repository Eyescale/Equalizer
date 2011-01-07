
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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

#include "canvas.h"
#include "channel.h"
#include "compound.h"
#include "config.h"
#include "configDestCompoundVisitor.h"
#include "layout.h"
#include "observer.h"
#include "segment.h"

#include <eq/fabric/paths.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{
typedef  fabric::View< Layout, View, Observer > Super;

View::View( Layout* parent )
        : Super( parent )
{
}

View::~View()
{
    // Use copy - Channel::unsetOutput modifies vector
    Channels channels = _channels;
    for( Channels::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        Channel* channel = *i;
        channel->unsetOutput();
    }

    EQASSERT( _channels.empty( ));
    _channels.clear();
}

namespace
{
class FrustumUpdater : public ConfigVisitor
{
public:
    FrustumUpdater( const Channels& channels ) : _channels( channels ) {}
    virtual ~FrustumUpdater() {}

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
    const Channels& _channels;
};

class CapabilitiesUpdater : public ConfigVisitor
{
public:
    CapabilitiesUpdater( View* view )
            : _view( view )
            , _capabilities( _view->getMaximumCapabilities( ))
        {}

    virtual ~CapabilitiesUpdater(){}

    virtual VisitorResult visit( Compound* compound )
    {
        const Channel* dest = compound->getInheritChannel();
        if( !dest || dest->getView() != _view )
            return TRAVERSE_CONTINUE;

        const Channel* src = compound->getChannel();
        if( !src->supportsView( _view ))
            return TRAVERSE_CONTINUE;

        const uint64_t supported = src->getCapabilities();
        _capabilities &= supported;
        return TRAVERSE_CONTINUE;
    }

    uint64_t getCapabilities() const { return _capabilities; }

private:
    View* const _view;
    uint64_t _capabilities;
};

}

void View::setDirty( const uint64_t bits )
{
    if( bits == 0 || !isAttached( ))
        return;

    Super::setDirty( bits );
    _updateChannels();
}

void View::_updateChannels() const
{
    EQASSERT( isMaster( ));
    co::ObjectVersion version( this );
    if( isDirty( ))
        ++version.version;
        
    for( Channels::const_iterator i = _channels.begin();
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        channel->setViewVersion( version );
    }
}

void View::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    EQASSERT( isMaster( ));
    Super::deserialize( is, dirtyBits );

    if( dirtyBits & ( DIRTY_FRUSTUM | DIRTY_OVERDRAW ))
    {
        const Channels& channels = getChannels();
        Config* config = getConfig();
        EQASSERT( config );

        FrustumUpdater updater( channels );
        config->accept( updater );
    }
}

Config* View::getConfig()
{
    Layout* layout = getLayout();
    EQASSERT( layout );
    return layout ? layout->getConfig() : 0;
}

const Config* View::getConfig() const
{
    const Layout* layout = getLayout();
    EQASSERT( layout );
    return layout ? layout->getConfig() : 0;
}

ServerPtr View::getServer()
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

void View::addChannel( Channel* channel )
{
    _channels.push_back( channel );
}

bool View::removeChannel( Channel* channel )
{
    Channels::iterator i = stde::find( _channels, channel );

    EQASSERT( i != _channels.end( ));
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    return true;
}

ViewPath View::getPath() const
{
    const Layout* layout = getLayout();
    EQASSERT( layout );
    ViewPath path( layout->getPath( ));
    
    const Views& views = layout->getViews();
    Views::const_iterator i = std::find( views.begin(), views.end(), this );
    EQASSERT( i != views.end( ));
    path.viewIndex = std::distance( views.begin(), i );
    return path;
}

void View::trigger( const Canvas* canvas, const bool active )
{
    const Mode mode = getMode();
    Config* config = getConfig();

    // (De)activate destination compounds for canvas/eye(s)
    for( Channels::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        const Canvas* channelCanvas = channel->getCanvas();
        const Layout* canvasLayout = channelCanvas->getActiveLayout();
        if( (canvas && channelCanvas != canvas) ||
            (!canvas && canvasLayout != getLayout( )) )
        {
            continue;
        }

        const Segment* segment = channel->getSegment();
        const uint32_t segmentEyes = segment->getEyes();
        const uint32_t eyes = ( mode == MODE_MONO ) ?
                           EYE_CYCLOP & segmentEyes : EYES_STEREO & segmentEyes;
        if( eyes == 0 )
            continue;

        ConfigDestCompoundVisitor visitor( channel, true /*activeOnly*/ );
        config->accept( visitor );     

        const Compounds& compounds = visitor.getResult();
        for( Compounds::const_iterator j = compounds.begin(); 
             j != compounds.end(); ++j )
        {
            Compound* compound = *j;
            if( active )
                compound->activate( eyes );
            else
                compound->deactivate( eyes );
        }
    }
}

void View::activateMode( const Mode mode )
{
    if( getMode() == mode )
        return;

    Config* config = getConfig();
    if( config->isRunning( ))
    {
        config->postNeedsFinish();
        trigger( 0, false );
    }

    Super::activateMode( mode );

    if( config->isRunning( ))
        trigger( 0, true );
}

void View::updateCapabilities()
{
    CapabilitiesUpdater visitor( this );
    getConfig()->accept( visitor );
    setCapabilities( visitor.getCapabilities( ));
}

}
}

#include "../fabric/view.ipp"

template class eq::fabric::View< eq::server::Layout, eq::server::View,
                                 eq::server::Observer >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                         const eq::fabric::View< eq::server::Layout,
                                                 eq::server::View,
                                                 eq::server::Observer >& );
/** @endcond */
