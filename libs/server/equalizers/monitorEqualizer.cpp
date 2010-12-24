
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
#include <co/base/debug.h>
#include <vmmlib/vmmlib.hpp>

using namespace co::base;
using namespace std;

namespace eq
{
namespace server
{

namespace
{
class OutputFrameFinder : public CompoundVisitor
{
public:
    OutputFrameFinder( const std::string& name  )
                             : _frame(0)
                             , _name( name )  {}

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
    EQINFO << "New monitor equalizer @" << (void*)this << endl;
}

MonitorEqualizer::MonitorEqualizer( const MonitorEqualizer& from )
        : Equalizer( from )
{}

MonitorEqualizer::~MonitorEqualizer()
{
    attach( 0 );
    EQINFO << "Delete monitor equalizer @" << (void*)this << endl;
}

void MonitorEqualizer::attach( Compound* compound )
{
    _outputFrames.clear();
    _viewports.clear();
    Equalizer::attach( compound );
}

void MonitorEqualizer::notifyUpdatePre( Compound* compound, 
                                        const uint32_t frameNumber )
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
        _viewports.push_back( eq::Viewport::FULL );
        
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
    EQASSERTINFO( size == _outputFrames.size(), 
                  size << " != " << _outputFrames.size( ));
    EQASSERT( size == _viewports.size( ));

    for( size_t i = 0; i < size; ++i )
    {
        Frame* frame = inputFrames[ i ];
        Frame* outputFrame = _outputFrames[ i ];
        if( !outputFrame )
            continue;

        const Compound* srcCompound = outputFrame->getCompound();
        const Viewport& viewport = _viewports[ i ];

        // compute and apply input frame offset
        const int32_t offsetX = static_cast< int32_t >(
            static_cast< float >( pvp.w ) * viewport.x );
        const int32_t offsetY = static_cast< int32_t >(
            static_cast< float >( pvp.h ) * viewport.y );
        frame->setOffset( Vector2i( offsetX, offsetY ));
        
        // compute and apply output frame zoom
        const int32_t width   = static_cast< int32_t >(
            static_cast< float >( pvp.w ) * viewport.w );
        const int32_t height  = static_cast< int32_t >(
            static_cast< float >( pvp.h ) * viewport.h ) ; 

        const PixelViewport& srcPVP( srcCompound->getInheritPixelViewport( ));
        const float factorW = static_cast< float >( width ) / 
                              static_cast< float >( srcPVP.w );
        const float factorH = static_cast< float >( height ) / 
                              static_cast< float >( srcPVP.h );
            
        const Zoom newZoom( factorW, factorH );
            
        outputFrame->setZoom( newZoom );
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
