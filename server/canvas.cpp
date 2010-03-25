
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

#include "canvas.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "layout.h"
#include "log.h"
#include "nameFinder.h"
#include "node.h"
#include "pipe.h"
#include "segment.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>


using namespace eq::base;

namespace eq
{
namespace server
{

Canvas::Canvas()
        : _config( 0 )
        , _activeLayout( 0 )
{}

Canvas::Canvas( const Canvas& from, Config* config )
        : eq::Frustum( from )
        , _config( config )
        , _activeLayout( from._activeLayout )
{
    EQASSERT( config );

    for( SegmentVector::const_iterator i = from._segments.begin();
         i != from._segments.end(); ++i )
    {
        new Segment( **i, this );
    }

    for( LayoutVector::const_iterator i = from._layouts.begin();
         i != from._layouts.end(); ++i )
    {
        const Layout* layout = *i;
        if( layout )
        {
            const LayoutPath path( layout->getPath( ));
            addLayout( config->getLayout( path ));
        }
        else
            addLayout( 0 );
    }

    config->addCanvas( this );
    EQASSERT( _config );
}

Canvas::~Canvas()
{
    while( !_segments.empty( ))
    {
        Segment* segment = _segments.back();
        _removeSegment( segment );
        delete segment;
    }
    _layouts.clear();

    if( _config )
        _config->removeCanvas( this );

    _config = 0;
    _activeLayout = 0;
}

void Canvas::getInstanceData( net::DataOStream& os )
{
    // This function is overwritten from eq::Object, since the class is
    // intended to be subclassed on the client side. When serializing a
    // server::Canvas, we only transmit the effective bits, not all since that
    // potentially includes bits from subclassed eq::Canvases.
    const uint64_t dirty = eq::Canvas::DIRTY_CUSTOM - 1;
    os << dirty;
    serialize( os, dirty );
}

void Canvas::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & eq::Canvas::DIRTY_LAYOUT )
        os << _activeLayout;

    if( dirtyBits & eq::Canvas::DIRTY_CHILDREN )
    {
        for( SegmentVector::const_iterator i = _segments.begin(); 
             i != _segments.end(); ++i )
        {
            Segment* segment = *i;
            EQASSERT( segment->getID() != EQ_ID_INVALID );
            os << segment->getID();
        }
        os << EQ_ID_INVALID;

        for( LayoutVector::const_iterator i = _layouts.begin(); 
             i != _layouts.end(); ++i )
        {
            Layout* layout = *i;
            if( layout )
            {
                EQASSERT( layout->getID() != EQ_ID_INVALID );
                os << layout->getID();
            }
            else
                os << EQ_ID_NONE;
        }
        os << EQ_ID_INVALID;
    }
}

void Canvas::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & eq::Canvas::DIRTY_LAYOUT )
    {
        uint32_t index;
        is >> index;
        _useLayout( index );
    }
}

Segment* Canvas::getSegment( const SegmentPath& path )
{
    EQASSERTINFO( _segments.size() > path.segmentIndex,
                  _segments.size() << " <= " << path.segmentIndex );

    if( _segments.size() <= path.segmentIndex )
        return 0;

    return _segments[ path.segmentIndex ];
}

CanvasPath Canvas::getPath() const
{
    EQASSERT( _config );

    const CanvasVector& canvases = _config->getCanvases();
    CanvasVector::const_iterator i = std::find( canvases.begin(),
                                                 canvases.end(), this );
    EQASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

void Canvas::_addSegment( Segment* segment )
{
    EQASSERT( segment );
    EQASSERT( segment->getCanvas() == this );
    EQASSERT( std::find( _segments.begin(), _segments.end(), segment ) == 
              _segments.end( ));
    _segments.push_back( segment );
}

bool Canvas::_removeSegment( Segment* segment )
{
    SegmentVector::iterator i = find( _segments.begin(), _segments.end(), 
                                      segment );
    if( i == _segments.end( ))
        return false;

    EQASSERT( segment->getCanvas() == this );
    _segments.erase( i );
    return true;
}


Segment* Canvas::findSegment( const std::string& name )
{
    SegmentFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Canvas::addLayout( Layout* layout )
{
    EQASSERT( std::find( _layouts.begin(), _layouts.end(), layout ) == 
              _layouts.end( ));

    // dest channel creation is done be Config::addCanvas
    _layouts.push_back( layout );
}

void Canvas::_useLayout( const uint32_t index )
{
    if( _config && _config->isRunning( ))
        _switchLayout( _activeLayout, index );

    _activeLayout = index;
}

void Canvas::init()
{
    _switchLayout( EQ_ID_NONE, _activeLayout );
}

void Canvas::exit()
{
    _switchLayout( _activeLayout, EQ_ID_NONE );
}

namespace
{
class ActivateVisitor : public ConfigVisitor
{
public:
    ActivateVisitor( const ChannelVector& channels ) : _channels( channels ) {}
    virtual ~ActivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->activate();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    ChannelVector _channels;
};

class DeactivateVisitor : public ConfigVisitor
{
public:
    DeactivateVisitor( ChannelVector& channels )
            : _channels( channels ) {}
    virtual ~DeactivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->deactivate();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    ChannelVector& _channels;
};
}

void Canvas::_switchLayout( const uint32_t oldIndex, const uint32_t newIndex )
{
    EQASSERT( _config );
    if( oldIndex == newIndex )
        return;

    const size_t nLayouts = _layouts.size();
    const Layout* oldLayout = (oldIndex >= nLayouts) ? 0 :_layouts[oldIndex];
    const Layout* newLayout = (newIndex >= nLayouts) ? 0 :_layouts[newIndex];

    for( SegmentVector::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        const Segment* segment = *i;        
        const ChannelVector& destChannels = segment->getDestinationChannels();

        if( newLayout )
        {
            // activate channels used by new layout
            ChannelVector usedChannels;
            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == newLayout )
                    usedChannels.push_back( channel );
            }
            
            ActivateVisitor activator( usedChannels );
            _config->accept( activator );
        }

        if( oldLayout )
        {
            // de-activate channels used by old layout
            ChannelVector usedChannels;

            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == oldLayout )
                    usedChannels.push_back( channel );
            }
            DeactivateVisitor deactivator( usedChannels );
            _config->accept( deactivator );
        }
    }
}

namespace
{
template< class C >
VisitorResult _accept( C* canvas, CanvasVisitor& visitor )
{
    VisitorResult result = visitor.visitPre( canvas );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const SegmentVector& segments = canvas->getSegments();
    for( SegmentVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( canvas ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

VisitorResult Canvas::accept( CanvasVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Canvas::accept( CanvasVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Canvas::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        
        session->unmapObject( segment );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    session->unmapObject( this );
}

std::ostream& operator << ( std::ostream& os, const Canvas* canvas )
{
    if( !canvas )
        return os;
    
    os << disableFlush << disableHeader << "canvas" << std::endl;
    os << "{" << std::endl << indent; 

    const std::string& name = canvas->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const LayoutVector& layouts = canvas->getLayouts();
    for( LayoutVector::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
    {
        Layout* layout = *i;
        if( layout )
        {
            const Config*      config     = layout->getConfig();
            const std::string& layoutName = layout->getName();
            if( layoutName.empty() || 
                config->find< Layout >( layoutName ) != layout )
                os << layout->getPath() << std::endl;
            else
                os << "layout   \"" << layout->getName() << "\"" << std::endl;
        }
        else
            os << "layout   OFF" << std::endl;
    }

    os << static_cast< const eq::Frustum& >( *canvas );

    const SegmentVector& segments = canvas->getSegments();
    for( SegmentVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        os << **i;
    }
    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
