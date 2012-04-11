
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "canvasPackets.h"
#include "elementVisitor.h"
#include "log.h"
#include "nameFinder.h"
#include "paths.h"
#include "segment.h"

#include <co/command.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

template< class CFG, class C, class S, class L >
Canvas< CFG, C, S, L >::Canvas( CFG* config )
        : _config( config )
{
    LBASSERT( config );
    config->_addCanvas( static_cast< C* >( this ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class CFG, class C, class S, class L >
Canvas< CFG, C, S, L >::~Canvas()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    while( !_segments.empty( ))
    {
        S* segment = _segments.back();
        _removeChild( segment );
        release( segment );
    }

    _data.activeLayout = 0;
    _layouts.clear();
    _config->_removeCanvas( static_cast< C* >( this ));
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::backup()
{
    _backup = _data;
    Object::backup();
    Frustum::backup();
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::restore()
{
    Frustum::restore();
    Object::restore();
    activateLayout( _backup.activeLayout );
    _data = _backup;
    setDirty( DIRTY_LAYOUT );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::attach( const UUID& id,
                                     const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _config->getMainThreadQueue();
    LBASSERT( queue );

    registerCommand( CMD_CANVAS_NEW_SEGMENT, 
                     CmdFunc( this, &Canvas< CFG, C, S, L >::_cmdNewSegment ),
                     queue );
    registerCommand( CMD_CANVAS_NEW_SEGMENT_REPLY, 
                  CmdFunc( this, &Canvas< CFG, C, S, L >::_cmdNewSegmentReply ),
                     0 );
}

template< class CFG, class C, class S, class L >
uint128_t Canvas< CFG, C, S, L >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_SEGMENTS ))
        commitChildren< S, CanvasNewSegmentPacket >( _segments, incarnation );
    return Object::commit( incarnation );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::serialize( co::DataOStream& os,
                                        const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        os << _data.activeLayout;
    if( dirtyBits & DIRTY_SEGMENTS && isMaster( ))
        os.serializeChildren( _segments );
    if( dirtyBits & DIRTY_LAYOUTS )
        os.serializeChildren( _layouts );
    if( dirtyBits & DIRTY_FRUSTUM )
        Frustum::serialize( os );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::deserialize( co::DataIStream& is,
                                          const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
    {
        uint32_t index;
        is >> index;
        activateLayout( index );
    }

    if( dirtyBits & DIRTY_SEGMENTS )
    {
        if( isMaster( ))
            syncChildren( _segments );
        else
        {
            Segments result;
            is.deserializeChildren( this, _segments, result );
            _segments.swap( result );
            LBASSERT( _segments.size() == result.size( ));
        }
    }

    if( dirtyBits & DIRTY_LAYOUTS )
    {
        _layouts.clear();
        co::ObjectVersions layouts;
        is >> layouts;
        for( co::ObjectVersions::const_iterator i = layouts.begin();
             i != layouts.end(); ++i )
        {
            const UUID& id = (*i).identifier;

            if( id == UUID::ZERO )
                _layouts.push_back( 0 );
            else
            {
                L* layout = 0;
                _config->find( id, &layout );
                LBASSERT( layout );
                _layouts.push_back( layout );
            }
        }

        _config->updateCanvas( static_cast< C* >( this ));
    }
    if( dirtyBits & DIRTY_FRUSTUM )
        Frustum::deserialize( is );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( CFG::DIRTY_CANVASES );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::create( S** segment )
{
    *segment = _config->getServer()->getNodeFactory()->createSegment( 
        static_cast< C* >( this ));
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::release( S* segment )
{
    _config->getServer()->getNodeFactory()->releaseSegment( segment );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::notifyDetach()
{
    Object::notifyDetach();
    releaseChildren< C, S >( _segments );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::_addChild( S* segment )
{
    LBASSERT( segment );
    LBASSERT( segment->getCanvas() == this );
    _segments.push_back( segment );
    setDirty( DIRTY_SEGMENTS );
}

template< class CFG, class C, class S, class L >
bool Canvas< CFG, C, S, L >::_removeChild( S* segment )
{
    typename Segments::iterator i = stde::find( _segments, segment );
    if( i == _segments.end( ))
        return false;

    LBASSERT( segment->getCanvas() == this );
    _segments.erase( i );
    setDirty( DIRTY_SEGMENTS );
    if( isAttached() && !isMaster( ))
        postRemove( segment );
    return true;
}

template< class CFG, class C, class S, class L >
bool Canvas< CFG, C, S, L >::_mapViewObjects()
{
    return static_cast< typename CFG::Super* >( _config )->mapViewObjects();
}

template< class CFG, class C, class S, class L >
CanvasPath Canvas< CFG, C, S, L >::getPath() const
{
    const CFG* config = getConfig();
    LBASSERT( config );

    const Canvases& canvases = config->getCanvases();
    typename Canvases::const_iterator i = std::find( canvases.begin(),
                                                     canvases.end(),
                                                     this );
    LBASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

template< class CFG, class C, class S, class L >
S* Canvas< CFG, C, S, L >::findSegment( const std::string& name )
{
    NameFinder< S, Visitor > finder( name );
    accept( finder );
    return finder.getResult();
}

template< class CFG, class C, class S, class L >
const S* Canvas< CFG, C, S, L >::findSegment( const std::string& name ) const
{
    NameFinder< const S, Visitor > finder( name );
    accept( finder );
    return finder.getResult();
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::addLayout( L* layout )
{
    LBASSERT( stde::find( _layouts, layout ) == _layouts.end( ));

    // dest channel creation is done be Config::addCanvas
    _layouts.push_back( layout );
    setDirty( DIRTY_LAYOUTS );
}

template< class CFG, class C, class S, class L >
bool Canvas< CFG, C, S, L >::removeLayout( L* layout )
{
    typename Layouts::iterator i = stde::find( _layouts, layout );
    if( i == _layouts.end( ))
        return false;

    if( getActiveLayout() == layout )
    {
        _data.activeLayout = 0;
        setDirty( DIRTY_LAYOUT );
    }

    _layouts.erase( i );
    setDirty( DIRTY_LAYOUTS );
    return true;
}

template< class CFG, class C, class S, class L >
const L* Canvas< CFG, C, S, L >::getActiveLayout() const
{
    LBASSERTINFO( _data.activeLayout < _layouts.size(),
                  _data.activeLayout << " >= " << _layouts.size( ));
    return _layouts[ _data.activeLayout ];
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::useLayout( const uint32_t index )
{
    LBASSERT( index < _layouts.size( ));
    if( _data.activeLayout == index )
        return;

    _data.activeLayout = index;
    setDirty( DIRTY_LAYOUT );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::setSwapBarrier( SwapBarrierPtr barrier )
{
    if( barrier.isValid() && barrier->getName().empty( ))
    {
        const std::string& name = getName();
        std::stringstream out;
        out << "barrier.canvas.";
        if( name.empty( ))
            if( getConfig( ))
                out << getPath().canvasIndex;
            else
                out << (void*)this;
        else
            out << name;

        barrier->setName( out.str( ));
    }

    _swapBarrier = barrier; 
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* canvas, V& visitor )
{
    VisitorResult result = visitor.visitPre( canvas );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename C::Segments& segments = canvas->getSegments();
    for( typename C::Segments::const_iterator i = segments.begin(); 
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

template< class CFG, class C, class S, class L >
VisitorResult Canvas< CFG, C, S, L >::accept( Visitor& visitor )
{
    return _accept( static_cast< C* >( this ), visitor );
}

template< class CFG, class C, class S, class L >
VisitorResult Canvas< CFG, C, S, L >::accept( Visitor& visitor ) const
{
    return _accept( static_cast< const C* >( this ), visitor );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::setWall( const Wall& wall )
{
    if( getWall() == wall && getCurrentType() == TYPE_WALL )
        return;

    Frustum::setWall( wall );
    setDirty( DIRTY_FRUSTUM );
    for( typename Segments::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        (*i)->notifyFrustumChanged();
    }
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::setProjection( const Projection& projection )
{
    if( getProjection() == projection && getCurrentType() == TYPE_PROJECTION )
        return;

    Frustum::setProjection( projection );
    setDirty( DIRTY_FRUSTUM );
    for( typename Segments::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        (*i)->notifyFrustumChanged();
    }
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::unsetFrustum()
{
    if( getCurrentType() == TYPE_NONE )
        return;

    Frustum::unsetFrustum();
    setDirty( DIRTY_FRUSTUM );
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class CFG, class C, class S, class L > bool
Canvas< CFG, C, S, L >::_cmdNewSegment( co::Command& command )
{
    const CanvasNewSegmentPacket* packet =
        command.get< CanvasNewSegmentPacket >();
    
    S* segment = 0;
    create( &segment );
    LBASSERT( segment );

    getLocalNode()->registerObject( segment );
    segment->setAutoObsolete( _config->getLatency() + 1 );
    LBASSERT( segment->isAttached() );

    CanvasNewSegmentReplyPacket reply( packet );
    reply.segmentID = segment->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class CFG, class C, class S, class L > bool
Canvas< CFG, C, S, L >::_cmdNewSegmentReply( co::Command& command )
{
    const CanvasNewSegmentReplyPacket* packet =
        command.get< CanvasNewSegmentReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->segmentID );

    return true;
}

template< class CFG, class C, class S, class L >
std::ostream& operator << ( std::ostream& os, 
                            const Canvas< CFG, C, S, L >& canvas )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "canvas"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent; 

    const std::string& name = canvas.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const std::vector< L* >& layouts = canvas.getLayouts();
    for( typename std::vector< L* >::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
    {
        L* layout = *i;
        if( layout )
        {
            const CFG* config = layout->getConfig();
            const std::string& layoutName = layout->getName();
            const L* foundLayout = 0;
            config->find( layoutName, &foundLayout );
            if( foundLayout == layout )
                os << "layout   \"" << layoutName << "\"" << std::endl;
            else
                os << layout->getPath() << std::endl;
        }
        else
            os << "layout   OFF" << std::endl;
    }

    if( canvas.getSwapBarrier().isValid( ))
        os << *canvas.getSwapBarrier();
    os << static_cast< const Frustum& >( canvas );

    const std::vector< S* >& segments = canvas.getSegments();
    for( typename std::vector< S* >::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        os << **i;
    }
    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
