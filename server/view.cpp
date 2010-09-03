
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
#include "compoundActivateVisitor.h"
#include "config.h"
#include "configVisitor.h"
#include "findEyeDestCompoundVisitor.h"
#include "layout.h"
#include "observer.h"
#include "segment.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

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
class ViewUpdater : public ConfigVisitor
{
public:
    ViewUpdater( const Channels& channels ) : _channels( channels ) {}
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
    const Channels& _channels;
};
}

void View::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    //EQINFO << getVersion() << base::backtrace << std::endl;
    EQASSERT( isMaster( ));
    Super::deserialize( is, dirtyBits );
    setDirty( dirtyBits ); // redistribute slave changes

    if( dirtyBits & ( DIRTY_FRUSTUM | DIRTY_OVERDRAW ))
    {
        const Channels& channels = getChannels();
        Config* config = getConfig();
        EQASSERT( config );

        ViewUpdater updater( channels );
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
        const Channel* channel = *i;
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

        Compounds compounds;
        ConfigDestCompoundVisitor visitor( channel, compounds );
        config->accept( visitor );     
        for( Compounds::const_iterator j = compounds.begin(); 
             j != compounds.end(); ++j )
        {       
            Compound* compound = *j;
            _updateCompound( compound, active, eyes );
        }
    }
}

void View::activateMode( const Mode mode )
{
    if( getMode() == mode )
        return;

    const Config* config = getConfig();
    if( config->isStopped( ))
        return;

    trigger( 0, false );
    Super::activateMode( mode );
    trigger( 0, true );
}

void View::_updateCompound( Compound* compound, const bool activate, 
                            const uint32_t eyes )
{
    for( size_t i = 0; i < NUM_EYES; ++i )
    {
        const uint32_t eye = 1 << i;
        if( ( eyes & eye ) == 0 )
            continue;
            
        EQASSERT( compound->isDestination( ));

        CompoundActivateVisitor activator(  activate, 
                                     static_cast<eq::fabric::Eye>( eye ) );
        compound->accept( activator );
    }
}

}
}

#include "../lib/fabric/view.ipp"

template class eq::fabric::View< eq::server::Layout, eq::server::View,
                                 eq::server::Observer >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                         const eq::fabric::View< eq::server::Layout,
                                                 eq::server::View,
                                                 eq::server::Observer >& );
/** @endcond */
