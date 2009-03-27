
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

#ifndef EQSERVER_CONFIGADDCANVASVISITOR_H
#define EQSERVER_CONFIGADDCANVASVISITOR_H

namespace eq
{
namespace server
{

class ChannelViewFinder : public ConfigVisitor
{
public:
    ChannelViewFinder( const Segment* const segment, const View* const view ) 
            : _segment( segment ), _view( view ), _result( 0 ) {}

    virtual ~ChannelViewFinder(){}

    virtual VisitorResult visit( Channel* channel )
        {
            if( channel->getView() != _view )
                return TRAVERSE_CONTINUE;

            if( channel->getSegment() != _segment )
                return TRAVERSE_CONTINUE;

            _result = channel;
            return TRAVERSE_TERMINATE;
        }

    Channel* getResult() { return _result; }

private:
    const Segment* const _segment;
    const View* const    _view;
    Channel*             _result;
};

class ConfigAddCanvasVisitor : public ConfigVisitor
{
public:
    ConfigAddCanvasVisitor( Canvas* canvas, Config* config  )  
            : _canvas( canvas )
            , _config( config )
        {}
    virtual ~ConfigAddCanvasVisitor() {}

    virtual VisitorResult visit( View* view )
        {
            _view = view;
            _canvas->accept( *this );
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( Canvas* canvas )
        {
            if( canvas != _canvas ) // only consider our canvas
                return TRAVERSE_PRUNE;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visit( Segment* segment )
        {
            Viewport viewport = segment->getViewport();
            viewport.intersect( _view->getViewport( ));

            if( !viewport.hasArea())
            {
                EQLOG( LOG_VIEW )
                    << "View " << _view->getName() << _view->getViewport()
                    << " doesn't intersect " << segment->getName()
                    << segment->getViewport() << std::endl;
                
                return TRAVERSE_CONTINUE;
            }
                      
            Channel* segmentChannel = segment->getChannel( );
            if (!segmentChannel)
            {
                EQWARN << "Segment " << segment->getName()
                       << " has no output channel" << std::endl;
                return TRAVERSE_CONTINUE;
            }

            // try to reuse channel
            ChannelViewFinder finder( segment, _view );
            _view->getConfig()->accept( finder );
            Channel* channel = finder.getResult();

            if( !channel ) // create and add new channel
            {
                Window* window = segmentChannel->getWindow();
                channel = new Channel( *segmentChannel, window );

                channel->setOutput( _view, segment );
            }

            //----- compute channel viewport:
            // segment/view intersection in canvas space...
            Viewport contribution = viewport;
            // ... in segment space...
            contribution.transform( segment->getViewport( ));
            
             // segment output area
            Viewport subViewport = segmentChannel->getViewport();
            // ...our part of it    
            subViewport.apply( contribution );
            
            channel->setViewport( subViewport );
            
            EQLOG( LOG_VIEW ) 
                << "View @" << (void*)_view << ' ' << _view->getViewport()
                << " intersects " << segment->getName()
                << segment->getViewport() << " at " << subViewport
                << " using channel @" << (void*)channel << std::endl;

            return TRAVERSE_CONTINUE;
        }

protected:
    Canvas* const _canvas;
    Config* const _config; // For find channel
    View*         _view; // The current view
};

}
}
#endif // EQSERVER_CONFIGADDCANVASVISITOR_H
