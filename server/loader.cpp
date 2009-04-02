
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "loader.h" 

#include "canvas.h" 
#include "compound.h" 
#include "config.h" 
#include "layout.h" 
#include "segment.h" 
#include "serverVisitor.h" 
#include "view.h" 

using namespace std;

namespace eq
{
namespace server
{

namespace
{
class UnusedOutputChannelFinder : public ConfigVisitor
{
public:
    UnusedOutputChannelFinder() : _candidate( 0 ) {}

    virtual VisitorResult visit( Channel* channel )
        {
            if( _candidate ) // testing a candidate (see below)
                return TRAVERSE_CONTINUE;

            const View* view = channel->getView();
            if( !view )
                return TRAVERSE_CONTINUE;

            // see if it is used as a destination channel
            _candidate = channel;
            Config* config = channel->getConfig();
            config->accept( *this );

            if( _candidate ) // survived - not a destination channel yet
                _channels.push_back( _candidate );
            _candidate = 0;

            return TRAVERSE_CONTINUE;
        };

    virtual VisitorResult visit( Compound* compound )
        {
            if( !_candidate ) // not testing a candidate (see above)
                return TRAVERSE_PRUNE;
            
            const Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;

            if( _candidate == channel )
            {
                _candidate = 0; // channel already used
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_PRUNE; // only check destination channels
        }

    const ChannelVector& getResult() const { return _channels; }

private:
    Channel* _candidate;
    ChannelVector _channels;
};

}

void Loader::addOutputCompounds( ServerPtr server )
{
    const ConfigVector& configs = server->getConfigs();
    for( ConfigVector::const_iterator i = configs.begin(); 
         i != configs.end(); ++i )
    {
        UnusedOutputChannelFinder finder;
        Config* config = *i;
        config->accept( finder );

        const ChannelVector& channels = finder.getResult();
        if( channels.empty( ))
            continue;

        Compound* group = new Compound;
        config->addCompound( group );

        for( ChannelVector::const_iterator j = channels.begin(); 
             j != channels.end(); ++j )
        {
            Compound* compound = new Compound;
            group->addChild( compound );

            Channel* channel = *j;
            compound->setChannel( channel );
        }
    }
}

namespace
{
static void _addDestinationViews( Compound* compound )
{
    Channel* channel = compound->getChannel();
    
    if( channel ) // stand-alone channel
    {
        if( channel->getView( ))
            return;
        
        Layout* layout = new Layout;
        View*   view   = new View;
        *static_cast< eq::Frustum* >( view ) = compound->getFrustum();
        layout->addView( view );
        
        Canvas* canvas = new Canvas;
        canvas->useLayout( layout );
        
        Segment* segment = new Segment;
        segment->setChannel( channel );
        canvas->addSegment( segment );
        
        Config* config = compound->getConfig();
        config->addLayout( layout );
        config->addCanvas( canvas );
        
        Channel* newChannel = config->findChannel( segment, view );
        EQASSERT( newChannel );
        
        compound->setChannel( newChannel );
        compound->setViewport( Viewport::FULL );
        
        return;
    }
       
    // segment group
    CompoundVector segments;
    const CompoundVector& children = compound->getChildren();
    for( CompoundVector::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        Compound* child = *i;
        Channel* childChannel = child->getChannel();
        if( childChannel )
        {
            if( !childChannel->getView( ))
                segments.push_back( child );
        }
        else
            _addDestinationViews( child );
    }
    
    if( segments.empty( ))
        return;

    Layout* layout = new Layout;
    View*   view   = new View;
    layout->addView( view );
        
    Canvas* canvas = new Canvas;
    canvas->useLayout( layout );
    *static_cast< eq::Frustum* >( canvas ) = compound->getFrustum();
    
    for( CompoundVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        Compound* child = *i;
        Segment* segment = new Segment;

        segment->setChannel( child->getChannel( ));
        segment->setViewport( child->getViewport( ));
        *static_cast< eq::Frustum* >( segment ) = child->getFrustum();

        canvas->addSegment( segment );
    }

    Config* config = compound->getConfig();
    config->addLayout( layout );
    config->addCanvas( canvas );

    for( size_t i = 0; i < segments.size(); ++i )
    {
        Segment* segment = canvas->getSegments()[ i ];
        Channel* newChannel = config->findChannel( segment, view );
        EQASSERT( newChannel );
        
        segments[i]->setChannel( newChannel );
        segments[i]->setViewport( Viewport::FULL );
    }
}

class AddDestinationViewVisitor : public ServerVisitor
{
    virtual VisitorResult visit( Compound* compound )
        {
            _addDestinationViews( compound );
            return TRAVERSE_PRUNE;
        }
};

}

void Loader::addDestinationViews( ServerPtr server )
{
    AddDestinationViewVisitor visitor;
    server->accept( visitor );
}

}
}
