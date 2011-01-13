
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "loader.h" 

#include "canvas.h" 
#include "compound.h" 
#include "connectionDescription.h" 
#include "config.h" 
#include "layout.h" 
#include "node.h" 
#include "observer.h" 
#include "segment.h" 
#include "serverVisitor.h" 
#include "view.h" 

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

    const Channels& getResult() const { return _channels; }

private:
    Channel* _candidate;
    Channels _channels;
};

}

void Loader::addOutputCompounds( ServerPtr server )
{
    const Configs& configs = server->getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        UnusedOutputChannelFinder finder;
        Config* config = *i;
        config->accept( finder );

        const Channels& channels = finder.getResult();
        if( channels.empty( ))
            continue;

        Compound* group = new Compound( config );
        for( Channels::const_iterator j = channels.begin(); 
             j != channels.end(); ++j )
        {
            Compound* compound = new Compound( group );
            Channel* channel = *j;
            compound->setChannel( channel );
        }
    }
}

namespace
{
class ConvertTo11Visitor : public ServerVisitor
{
    virtual VisitorResult visitPre( Config* config )
    {
        const float version = config->getFAttribute( Config::FATTR_VERSION );
        if (  version >= 1.1f )
            return TRAVERSE_PRUNE;
        return TRAVERSE_CONTINUE;
    }
    virtual VisitorResult visitPost( Config* config )
    {
        config->setFAttribute( Config::FATTR_VERSION, 1.1f );
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visitPre( Node* node )
    {
        if( node->isApplicationNode() && 
            node->getConnectionDescriptions().empty() &&
            node->getConfig()->getNodes().size() > 1 )
        {
            //RFE 3156103: Add default appNode connection for multi-node configs
            EQINFO << "Adding default appNode connection for multi-node config"
                   << std::endl;
            node->addConnectionDescription( new ConnectionDescription );
        }
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visit( Segment* segment )
    {
        segment->setEyes( 0 ); // eyes will be re-enabled below
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visit( Compound* compound )
    {
        if( !compound->isDestination() )
            return TRAVERSE_CONTINUE;

        Channel* channel = compound->getChannel();
        Segment* segment = channel->getSegment();
        View* view = channel->getView();

        if( segment == 0 || view == 0 ) // view-less dest compound
            return TRAVERSE_PRUNE;

        uint32_t compoundEyes = compound->getEyes();
        const uint32_t segmentEyes = segment->getEyes();
        Compound* parent = compound->getParent();

        if( compoundEyes != fabric::EYE_UNDEFINED &&
            ( compoundEyes & fabric::EYE_CYCLOP ) == 0 )
        {
            view->changeMode( View::MODE_STEREO );
            compound->enableEye( fabric::EYE_CYCLOP );
        }
        while( compoundEyes == fabric::EYE_UNDEFINED && parent )
        {
            const uint32_t parentEyes = parent->getEyes();
            if( parentEyes == fabric::EYE_UNDEFINED )
            {
                parent = parent->getParent();
                continue;
            }
            compoundEyes = parentEyes;
            if( ( parentEyes & fabric::EYE_CYCLOP ) == 0 )
            {
                view->changeMode( View::MODE_STEREO );
                parent->enableEye( fabric::EYE_CYCLOP );
            }
            parent = parent->getParent();
        }

        if( compoundEyes == fabric::EYE_UNDEFINED )
            compoundEyes = fabric::EYES_ALL;

        segment->setEyes( compoundEyes | segmentEyes | fabric::EYE_CYCLOP );
        return TRAVERSE_PRUNE;
    }
};
}

void Loader::convertTo11( ServerPtr server )
{
    ConvertTo11Visitor visitor;
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
        EQASSERT( newChannel );
        
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
        EQASSERT( newChannel );
        
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
            EQASSERT( observers.size() == 1 );

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
