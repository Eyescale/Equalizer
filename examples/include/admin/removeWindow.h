
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution. Neither the name of Eyescale Software GmbH nor the
 * names of its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EQ_ADMIN_REMOVE_WINDOW_H
#define EQ_ADMIN_REMOVE_WINDOW_H

#include "findPipe.h"

#include <eq/admin/base.h>

namespace eqAdmin
{
namespace
{
class FindChannelSegments : public eq::admin::ConfigVisitor
{
public:
    FindChannelSegments( eq::admin::Channel* channel ) : _channel( channel ) {}
    virtual ~FindChannelSegments() {}

    virtual eq::admin::VisitorResult visit( eq::admin::Segment* segment )
        {
            if( segment->getChannel() == _channel )
                _segments.push_back( segment );
            return eq::admin::TRAVERSE_CONTINUE;
        }

    const eq::admin::Segments& getResult() const { return _segments; }

private:
    eq::admin::Channel* const _channel; 
    eq::admin::Segments _segments;
};

class FindCanvasLayouts : public eq::admin::ConfigVisitor
{
public:
    FindCanvasLayouts( const eq::admin::Canvas* canvas )
            : _canvas( canvas ), _layout( 0 ) {}
    virtual ~FindCanvasLayouts() {}

    virtual eq::admin::VisitorResult visitPre( eq::admin::Canvas* canvas )
        {
            if( canvas == _canvas && !_layout )
            {
                const eq::admin::Layouts& layouts = canvas->getLayouts();
                eq::admin::Config* config = canvas->getConfig();
                for( eq::admin::Layouts::const_iterator i = layouts.begin();
                     i != layouts.end(); ++i )
                {
                    _layout = *i;
                    if( _layout &&
                        config->accept(*this) == eq::admin::TRAVERSE_CONTINUE )
                    {
                        _layouts.push_back( _layout );
                    }
                }
                _layout = 0;
                return eq::admin::TRAVERSE_TERMINATE;
            }
            else if( canvas != _canvas && _layout )
            {
                const eq::admin::Layouts& layouts = canvas->getLayouts();
                if( stde::find( layouts, _layout ) != layouts.end( ))
                    return eq::admin::TRAVERSE_TERMINATE; // layout used by this canvas
            }
            return eq::admin::TRAVERSE_CONTINUE;
        }

    const eq::admin::Layouts& getResult() const { return _layouts; } 

private:
    const eq::admin::Canvas* const _canvas;
    eq::admin::Layout* _layout;
    eq::admin::Layouts _layouts;
};

class FindLayoutObservers : public eq::admin::ConfigVisitor
{
public:
    FindLayoutObservers( const eq::admin::Layout* layout )
            : _layout( layout ), _observer( 0 ) {}
    virtual ~FindLayoutObservers() {}

    virtual eq::admin::VisitorResult visit( eq::admin::View* view )
        {
            if( view->getLayout() == _layout && !_observer )
            {
                eq::admin::Config* config = view->getConfig();
                _observer = view->getObserver();
                if( _observer &&
                    config->accept(*this) == eq::admin::TRAVERSE_CONTINUE )
                {
                    _observers.push_back( _observer );
                    view->setObserver( 0 ); // observer will be deleted
                }
                _observer = 0;
                return eq::admin::TRAVERSE_TERMINATE;
            }
            else if( view->getLayout() != _layout && _observer )
            {
                if( _observer == view->getObserver( ))
                    return eq::admin::TRAVERSE_TERMINATE; // used by layout
            }
            return eq::admin::TRAVERSE_CONTINUE;
        }

    const eq::admin::Observers& getResult() const { return _observers; } 

private:
    const eq::admin::Layout* const _layout;
    eq::admin::Observer* _observer;
    eq::admin::Observers _observers;
};

}

inline bool removeWindow( eq::admin::ServerPtr server )
{
    if( !server )
       return false;

    // Find first pipe...
    eq::admin::Pipe* pipe = findPipe( server );
    if( !pipe )
       return false;

    const eq::admin::Windows& windows = pipe->getWindows();
    if( windows.size() < 2 )
        return false;

    // Remove last window (->channels->segment->canvas->layout->view)
    eq::admin::Window* window = windows.back();
    eq::admin::Config* config = pipe->getConfig();

    const eq::admin::Channels& channels = window->getChannels();
    for( eq::admin::Channels::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        // delete dependent segments
        eq::admin::Channel* channel = *i;
        FindChannelSegments channelSegments( channel );
        config->accept( channelSegments );
        const eq::admin::Segments& segments = channelSegments.getResult();

        for( eq::admin::Segments::const_iterator j = segments.begin();
             j != segments.end(); ++j )
        {
            eq::admin::Segment* segment = *j;
            eq::admin::Canvas* canvas = segment->getCanvas();
            delete segment;
            
            // delete now-empty canvases
            if( !canvas->getSegments().empty( ))
                continue;

            // delete now-empty layouts
            FindCanvasLayouts canvasLayouts( canvas );
            config->accept( canvasLayouts );
            const eq::admin::Layouts& layouts = canvasLayouts.getResult();
            for( eq::admin::Layouts::const_iterator k = layouts.begin();
                 k != layouts.end(); ++k )
            {
                eq::admin::Layout* layout = *k;
                FindLayoutObservers layoutObservers( layout );
                config->accept( layoutObservers );
                eq::admin::Observers observers = layoutObservers.getResult();
                stde::usort( observers );

                for( eq::admin::Observers::const_iterator l = observers.begin();
                     l != observers.end(); ++l )
                {
                    delete *l;
                }
                delete layout;
            }
            delete canvas;
        }
    }

    delete window;
    config->commit();
    return true;
}

}
#endif // EQ_ADMIN_REMOVE_WINDOW_H
