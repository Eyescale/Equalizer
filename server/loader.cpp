
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
class AddDestinationViewVisitor : public ServerVisitor
{
public:
    AddDestinationViewVisitor() : _canvas( 0 ), _compound( 0 ), _view( 0 ) {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( channel && channel->getView( ))
                return TRAVERSE_PRUNE;

            if( !_canvas && compound->getFrustumType() == Frustum::TYPE_NONE )
                return TRAVERSE_CONTINUE;

            Layout* layout;

            if( !_canvas )
            {
                layout = new Layout;

                if( !channel )
                {
                    _view = new View;
                    *static_cast< eq::Frustum* >( _view ) = 
                        compound->getFrustum();
                    layout->addView( _view );
                }

                _canvas = new Canvas;
                _canvas->useLayout( layout );
                
                _compound = compound;
            }
            else
                layout = _canvas->getLayout();

            if( channel )
            {
                if( compound->getFrustumType() != Frustum::TYPE_NONE )
                {
                    View* view = new View;
                    view->setViewport( compound->getViewport( ));
                    *static_cast< eq::Frustum* >( view ) = 
                        compound->getFrustum();
                    layout->addView( view );
                    
                    _views.push_back( view );
                }
                else
                    _views.push_back( _view );

                Segment* segment = new Segment;
                segment->setViewport( compound->getViewport() );
                segment->setChannel( channel );
                _canvas->addSegment( segment );

                _compounds.push_back( compound );
            }

            if( compound->isLeaf() || channel )
                _setup( compound );

            return channel ? TRAVERSE_PRUNE : TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPost( Compound* compound )
        { return _setup( compound ); }
    
private:
    Canvas* _canvas;
    Compound* _compound;
    CompoundVector _compounds;

    View* _view;
    ViewVector _views;

    VisitorResult _setup( Compound* compound )
        {
            if( compound != _compound )
                return TRAVERSE_CONTINUE;
            
            EQASSERT( _canvas );

            Config* config = compound->getConfig();
            Layout* layout = _canvas->getLayout( );

            config->addLayout( layout );
            config->addCanvas( _canvas );

            const SegmentVector& segments = _canvas->getSegments();

            EQASSERT( segments.size() == _views.size( ));
            EQASSERT( segments.size() == _compounds.size( ));

            for( size_t i = 0; i < segments.size(); ++i )
            {
                if( !_views[ i ] )
                    continue;

                Channel* channel = config->findChannel( segments[i], _views[i]);
                EQASSERT( channel );

                _compounds[i]->setChannel( channel );
                _compounds[i]->setViewport( Viewport::FULL );
            }

            _canvas = 0;
            _compound = 0;
            _compounds.clear();
            _view = 0;
            _views.clear();

            return TRAVERSE_CONTINUE;
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
