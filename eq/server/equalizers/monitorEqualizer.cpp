
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "monitorEqualizer.h"

#include "../compound.h"
#include "../config.h"
#include "../compoundVisitor.h"
#include "../log.h"
#include "../frame.h"
#include "../view.h"
#include "../segment.h"

#include <eq/fabric/viewport.h>
#include <eq/fabric/zoom.h>
#include <lunchbox/debug.h>

namespace eq
{
namespace server
{

namespace
{
class OutputFrameFinder : public CompoundVisitor
{
public:
    explicit OutputFrameFinder( const std::string& name  )
        : _frame(0) , _name( name )  {}

    virtual ~OutputFrameFinder(){}

    virtual VisitorResult visit( const Compound* compound )
    {
        const Frames& outputFrames = compound->getOutputFrames();
        for( Frames::const_iterator i = outputFrames.begin();
             i != outputFrames.end(); ++i )
        {
            Frame* frame = *i;

            if ( frame->getName() == _name )
            {
                _frame = frame;
                return TRAVERSE_TERMINATE;
            }
        }
        return TRAVERSE_CONTINUE;
    }

    Frame* getResult() { return _frame; }

private:
    Frame* _frame;
    const std::string& _name;
};
}

MonitorEqualizer::MonitorEqualizer()
{
    LBINFO << "New monitor equalizer @" << (void*)this << std::endl;
}

MonitorEqualizer::MonitorEqualizer( const MonitorEqualizer& from )
        : Equalizer( from )
{}

MonitorEqualizer::~MonitorEqualizer()
{
    attach( 0 );
    LBINFO << "Delete monitor equalizer @" << (void*)this << std::endl;
}

void MonitorEqualizer::attach( Compound* compound )
{
    _outputFrames.clear();
    _viewports.clear();
    Equalizer::attach( compound );
}

void MonitorEqualizer::notifyUpdatePre( Compound*, const uint32_t )
{
    _updateViewports();
    _updateZoomAndOffset();
}

void MonitorEqualizer::_updateViewports()
{
    if( !_outputFrames.empty( ))
        return;

    Compound* compound = getCompound();
    if( !compound )
        return;

    const Frames& inputFrames = compound->getInputFrames();
    for( Frames::const_iterator i = inputFrames.begin();
         i != inputFrames.end(); ++i )
    {
        const Frame* frame = *i;
        const Compound* root = compound->getRoot();

        // find the output frame
        OutputFrameFinder frameFinder( frame->getName() );
        root->accept( frameFinder );
        Frame* outputFrame = frameFinder.getResult();

        _outputFrames.push_back( outputFrame );
        _viewports.push_back( Viewport::FULL );

        if( outputFrame )
        {
            const Channel* channel = outputFrame->getChannel();
            const Segment* segment = channel->getSegment();
            const View* view =  channel->getView();

            if( view )
            {
                Viewport viewport( segment->getViewport( ));
                viewport.intersect( view->getViewport( ));

                _viewports.back() = viewport;
            }
        }
    }
}

void MonitorEqualizer::_updateZoomAndOffset()
{
    const Compound* compound( getCompound( ));
    const PixelViewport& pvp( compound->getInheritPixelViewport( ));

    const Frames& inputFrames( compound->getInputFrames( ));
    const size_t size( inputFrames.size( ));
    LBASSERTINFO( size == _outputFrames.size(),
                  size << " != " << _outputFrames.size( ));
    LBASSERT( size == _viewports.size( ));

    for( size_t i = 0; i < size; ++i )
    {
        Frame* frame = inputFrames[ i ];
        Frame* outputFrame = _outputFrames[ i ];
        if( !outputFrame )
            continue;

        const Compound* srcCompound = outputFrame->getCompound();
        const Viewport& viewport = _viewports[ i ];

        // compute and apply input frame offset
        const int32_t offsetX = int32_t( float( pvp.w ) * viewport.x );
        const int32_t offsetY = int32_t( float( pvp.h ) * viewport.y );
        frame->setNativeOffset( Vector2i( offsetX, offsetY ));

        // compute and apply output frame zoom
        const int32_t width   = int32_t( float( pvp.w ) * viewport.w );
        const int32_t height  = int32_t( float( pvp.h ) * viewport.h ) ;
        const PixelViewport& srcPVP( srcCompound->getInheritPixelViewport( ));
        const Zoom zoom( float( width ) / float( srcPVP.w ),
                         float( height ) / float( srcPVP.h ));
        outputFrame->setNativeZoom( zoom );
    }
}

std::ostream& operator << ( std::ostream& os, const MonitorEqualizer* equalizer)
{
    if( equalizer )
        os << "monitor_equalizer {}" << std::endl;
    return os;
}

}
}
