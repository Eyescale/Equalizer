
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "configVisitor.h"
#include "layout.h"
#include "observer.h"

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
    EQASSERT( isMaster( ));
    Super::deserialize( is, dirtyBits );
    setDirty( dirtyBits ); // redistribute slave changes

    if( dirtyBits & ( DIRTY_FRUSTUM | DIRTY_OVERDRAW ))
    {
        const Channels& channels = getChannels();
        Config*              config   = getConfig();
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
