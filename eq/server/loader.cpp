
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "loader.h" 

#include "canvas.h" 
#include "compound.h" 
#include "configVisitor.h"
#include "connectionDescription.h" 
#include "config.h" 
#include "global.h" 
#include "layout.h" 
#include "node.h" 
#include "observer.h" 
#include "segment.h" 
#include "view.h" 

#include <eq/fabric/elementVisitor.h>

namespace eq
{
namespace server
{
#include "convert11Visitor.h"
#include "convert12Visitor.h"

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

    const Channels& getResult() const { return _channels; }

private:
    Channel* _candidate;
    Channels _channels;
};

}

Compounds Loader::addOutputCompounds( ServerPtr server )
{
    Compounds result;

    const Configs& configs = server->getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        UnusedOutputChannelFinder finder;
        Config* config = *i;
        config->accept( finder );

        Channels channels = finder.getResult();
        while( !channels.empty( ))
        {
            const Layout* layout = channels.front()->getLayout();

            Compound* group = new Compound( config );
            for( ChannelsIter j = channels.begin(); j != channels.end(); )
            {
                Channel* channel = *j;
                if( channel->getLayout() == layout )
                {
                    Compound* compound = new Compound( group );
                    compound->setChannel( channel );
                    j = channels.erase( j );
                }
                else
                    ++j;
            }
            result.push_back( group );
        }
    }
    return result;
}

void Loader::convertTo11( ServerPtr server )
{
    ConvertTo11Visitor visitor;
    server->accept( visitor );
}

void Loader::convertTo12( ServerPtr server )
{
    ConvertTo12Visitor visitor;
    server->accept( visitor );
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
        
        Config* config = compound->getConfig();
        Layout* layout = new Layout( config );
        View*   view   = new View( layout );
        *static_cast< eq::Frustum* >( view ) = compound->getFrustum();
        
        Canvas* canvas = new Canvas( config );
        canvas->addLayout( layout );
        
        Segment* segment = new Segment( canvas );
        segment->setChannel( channel );
        
        config->activateCanvas( canvas );
        
        Channel* newChannel = config->findChannel( segment, view );
        LBASSERT( newChannel );
        
        compound->setChannel( newChannel );
        compound->setViewport( Viewport::FULL );
        
        return;
    }
       
    // segment group
    Compounds segments;
    const Compounds& children = compound->getChildren();
    for( Compounds::const_iterator i = children.begin();
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

    Config* config = compound->getConfig();
    Layout* layout = new Layout( config );
    View*   view   = new View( layout );        
    Canvas* canvas = new Canvas( config );

    canvas->addLayout( layout );
    *static_cast< eq::Frustum* >( canvas ) = compound->getFrustum();
    
    for( Compounds::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        Compound* child = *i;
        Segment* segment = new Segment( canvas );

        segment->setChannel( child->getChannel( ));
        segment->setViewport( child->getViewport( ));
        *static_cast< eq::Frustum* >( segment ) = child->getFrustum();
    }

    config->activateCanvas( canvas );

    for( size_t i = 0; i < segments.size(); ++i )
    {
        Segment* segment = canvas->getSegments()[ i ];
        Channel* newChannel = config->findChannel( segment, view );
        LBASSERT( newChannel );
        
        segments[i]->setChannel( newChannel );
        segments[i]->setViewport( Viewport::FULL );
    }
}

class AddDestinationViewVisitor : public ServerVisitor
{
    virtual VisitorResult visitPre( Config* config )
        {
            if( config->getCanvases().empty( ))
                return TRAVERSE_CONTINUE;

            return TRAVERSE_PRUNE; // Config has already canvases, ignore.
        }

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

namespace
{
class AddObserverVisitor : public ServerVisitor
{
    virtual VisitorResult visitPre( Config* config )
        {
            const Observers& observers = config->getObservers();
            if( !observers.empty( ))
                return TRAVERSE_PRUNE;

            new Observer( config );
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visit( View* view )
        {
            const Observers& observers = view->getConfig()->getObservers();
            LBASSERT( observers.size() == 1 );

            view->setObserver( observers.front( ));
            return TRAVERSE_CONTINUE; 
        }
};

}

void Loader::addDefaultObserver( ServerPtr server )
{
    AddObserverVisitor visitor;
    server->accept( visitor );
}

}
}
