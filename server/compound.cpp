
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"
#include "colorMask.h"
#include "compoundInitVisitor.h"
#include "compoundExitVisitor.h"
#include "compoundUpdateDataVisitor.h"
#include "compoundUpdateInputVisitor.h"
#include "compoundUpdateOutputVisitor.h"
#include "config.h"
#include "constCompoundVisitor.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "swapBarrier.h"

#include <eq/base/base.h>
#include <eq/base/stdExt.h>
#include <eq/client/colorMask.h>
#include <eq/client/global.h>
#include <eq/client/packets.h>
#include <eq/client/windowSystem.h>
#include <eq/net/session.h>

#include <algorithm>
#include <math.h>
#include <vector>

using namespace eqBase;
using namespace std;
using namespace stde;

namespace eqs
{
#define MAKE_ATTR_STRING( attr ) ( string("EQ_COMPOUND_") + #attr )
std::string Compound::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_STEREO_MODE ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_LEFT_MASK ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_RIGHT_MASK ),
    MAKE_ATTR_STRING( IATTR_UPDATE_FOV )
};

Compound::Compound()
        : _config( 0 )
        , _parent( 0 )
        , _swapBarrier( 0 )
{
}

// copy constructor
Compound::Compound( const Compound& from )
        : PixelViewportListener()
        , _config( 0 )
        , _parent( 0 )
{
    _name        = from._name;
    _view        = from._view;
    _data        = from._data;
    _swapBarrier = from._swapBarrier;

    const uint32_t nCompounds = from.nChildren();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        const Compound* child = from.getChild(i);
        addChild( new Compound( *child ));
    }

    for( vector<Frame*>::const_iterator iter = from._outputFrames.begin();
         iter != from._outputFrames.end(); ++iter )

        addOutputFrame( new Frame( **iter ));

    for( vector<Frame*>::const_iterator iter = from._inputFrames.begin();
         iter != from._inputFrames.end(); ++iter )

        addInputFrame( new Frame( **iter ));
}

Compound::~Compound()
{
    if( _config )
        _config->removeCompound( this );

    _config = 0;

    for( vector<Compound*>::const_iterator i = _children.begin(); 
         i != _children.end(); ++i )
    {
        Compound* compound = *i;

        compound->_parent = 0;
        delete compound;
    }
    _children.clear();

    for( vector<Frame*>::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = 0;
        delete frame;
    }
    _inputFrames.clear();

    for( vector<Frame*>::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = 0;
        delete frame;
    }
    _outputFrames.clear();
}

Compound::InheritData::InheritData()
        : channel( 0 ),
          buffers( eq::Frame::BUFFER_UNDEFINED ),
          eyes( EYE_UNDEFINED ),
          tasks( TASK_DEFAULT ),
          period( EQ_UNDEFINED_UINT32 ),
          phase( EQ_UNDEFINED_UINT32 )
{
    const Global* global = Global::instance();
    for( int i=0; i<IATTR_ALL; ++i )
        iAttributes[i] =
            global->getCompoundIAttribute( static_cast< IAttribute >( i ));
}

void Compound::addChild( Compound* child )
{
    _children.push_back( child );
    EQASSERT( !child->_parent );
    child->_parent = this;
}

Compound* Compound::getNext() const
{
    if( !_parent )
        return 0;

    vector<Compound*>&          siblings = _parent->_children;
    vector<Compound*>::iterator result   = find( siblings.begin(),
                                                 siblings.end(), this);

    if( result == siblings.end() )
        return 0;
    result++;
    if( result == siblings.end() )
        return 0;

    return *result;
}

void Compound::setChannel( Channel* channel )
{ 
    if( _data.channel == channel )
        return;
    
    if( _data.channel )
        _data.channel->removePVPListener( this );
    
    _data.channel = channel;
    _initialPVP.invalidate();

    if( channel )
    {
        channel->addPVPListener( this );
        notifyPVPChanged( channel->getPixelViewport( ));
    }
}

const Channel* Compound::getChannel() const
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return 0;
}

Channel* Compound::getChannel()
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return 0;
}

eqs::Window* Compound::getWindow()
{
    Channel* channel = getChannel();
    if( channel )
        return channel->getWindow();
    return 0;
}

const eqs::Window* Compound::getWindow() const
{
    const Channel* channel = getChannel();
    if( channel )
        return channel->getWindow();
    return 0;
}

void Compound::setSwapBarrier( SwapBarrier* barrier )
{
    if( barrier && barrier->getName().empty( ))
    {
        const Compound* root     = getRoot();
        const string&   rootName = root->getName();
        if( rootName.empty( ))
            barrier->setName( "barrier" );
        else
            barrier->setName( "barrier." + rootName );
    }

    _swapBarrier = barrier; 
}

void Compound::addInputFrame( Frame* frame )
{ 
    EQASSERT( frame );
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _inputFrames.push_back( frame ); 
    frame->_compound = this;
}
void Compound::addOutputFrame( Frame* frame )
{ 
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _outputFrames.push_back( frame ); 
    frame->_compound = this;
}

void Compound::_setDefaultFrameName( Frame* frame )
{
    for( Compound* compound = this; compound; compound = compound->getParent())
    {
        if( !compound->getName().empty( ))
        {
            frame->setName( "frame." + compound->getName( ));
            return;
        }

        const Channel* channel = compound->getChannel();
        if( channel && !channel->getName().empty( ))
        {
            frame->setName( "frame." + channel->getName( ));
            return;
        }
    }
    frame->setName( "frame" );
}

//---------------------------------------------------------------------------
// view operations
//---------------------------------------------------------------------------
void Compound::setWall( const Wall& wall )
{
    _data.view.applyWall( wall );
    _view.wall   = wall;
    _view.latest = VIEW_WALL;
    _initialPVP.invalidate();

    if( _data.channel )
        notifyPVPChanged( _data.channel->getPixelViewport( ));
    EQVERB << "Wall: " << _data.view << endl;
}

void Compound::setProjection( const Projection& projection )
{
    _data.view.applyProjection( projection );
    _view.projection = projection;
    _view.latest     = VIEW_PROJECTION;
    _initialPVP.invalidate();

    if( _data.channel )
        notifyPVPChanged( _data.channel->getPixelViewport( ));
    EQVERB << "Projection: " << _data.view << endl;
}

void Compound::notifyPVPChanged( const eq::PixelViewport& pvp )
{
    if( !_initialPVP.isValid( )) // no valid channel pvp: set initial values
    {
        _initialPVP = pvp;
        return;
    }

    if( !_initialPVP.isValid( ) || !_data.view.isValid( ))
        return;
    
    const int32_t updateFOV = _inherit.iAttributes[ IATTR_UPDATE_FOV ];
    if( updateFOV == eq::OFF || updateFOV == eq::UNDEFINED )
        return;

    switch( _view.latest )
    {
        case VIEW_NONE:
            EQUNREACHABLE;
            return;

        case VIEW_WALL:
            switch( updateFOV )
            {
                case eq::HORIZONTAL:
                {
                    const float newAR = static_cast< float >( pvp.w ) /
                                        static_cast< float >( pvp.h );
                    const float initAR = static_cast< float >( _initialPVP.w ) /
                                         static_cast< float >( _initialPVP.h );
                    const float ratio  = newAR / initAR;

                    Wall wall( _view.wall );
                    wall.resizeHorizontal( ratio );
                    _data.view.applyWall( wall );
                    break;
                }
                case eq::VERTICAL:
                {
                    const float newAR = static_cast< float >( pvp.h ) /
                                        static_cast< float >( pvp.w );
                    const float initAR = static_cast< float >( _initialPVP.h ) /
                                         static_cast< float >( _initialPVP.w );
                    const float ratio  = newAR / initAR;

                    Wall wall( _view.wall );
                    wall.resizeVertical( ratio );
                    _data.view.applyWall( wall );
                    break;
                }
                default:
                    EQUNIMPLEMENTED;
            }
            break;

        case VIEW_PROJECTION:
            switch( updateFOV )
            {
                case eq::HORIZONTAL:
                {
                    const float newAR = static_cast< float >( pvp.w ) /
                                        static_cast< float >( pvp.h );
                    const float initAR = static_cast< float >( _initialPVP.w ) /
                                         static_cast< float >( _initialPVP.h );
                    const float ratio  = newAR / initAR;

                    Projection projection( _view.projection );
                    projection.resizeHorizontal( ratio );
                    _data.view.applyProjection( projection );
                    break;
                }
                case eq::VERTICAL:
                {
                    const float newAR = static_cast< float >( pvp.h ) /
                                        static_cast< float >( pvp.w );
                    const float initAR = static_cast< float >( _initialPVP.h ) /
                                         static_cast< float >( _initialPVP.w );
                    const float ratio  = newAR / initAR;

                    Projection projection( _view.projection );
                    projection.resizeVertical( ratio );
                    EQINFO << _view.projection << " -" << ratio << "-> "
                           << projection << endl;
                    _data.view.applyProjection( projection );
                    break;
                }
                default:
                    EQUNIMPLEMENTED;
            }
            break;

        default:
            EQUNIMPLEMENTED;
    }
}

//---------------------------------------------------------------------------
// accept
//---------------------------------------------------------------------------
template< class C, class V >
Compound::VisitorResult _accept( C* compound, V* visitor, 
                                 const bool activeOnly )
{
    if( compound->isLeaf( )) 
    {
        if ( !activeOnly || compound->isActive( )) 
            return visitor->visitLeaf( compound );
        return Compound::TRAVERSE_CONTINUE;
    }

    C* current = compound;
    while( true )
    {
        C* parent = current->getParent();
        C* next   = current->getNext();
        C* child  = (current->nChildren()) ? current->getChild(0) : 0;

        //---------- down-right traversal
        if ( !child ) // leaf
        {
            if ( !activeOnly || current->isActive( ))
            {
                if( visitor->visitLeaf( current ) ==
                    Compound::TRAVERSE_TERMINATE)

                    return Compound::TRAVERSE_TERMINATE;
            }

            current = next;
        } 
        else // node
        {
            if( !activeOnly || current->isActive( ))
            {
                if( visitor->visitPre( current ) == 
                    Compound::TRAVERSE_TERMINATE )

                    return Compound::TRAVERSE_TERMINATE;

                current = child;
            }
            else
                current = next;
        }

        //---------- up-right traversal
        if( !current && !parent ) return Compound::TRAVERSE_CONTINUE;

        while( !current )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->getNext();

            if( !activeOnly || current->isActive( ))
            {
                if( visitor->visitPost( current ) == 
                    Compound::TRAVERSE_TERMINATE)

                    return Compound::TRAVERSE_TERMINATE;
            }
            
            if ( current == compound ) return Compound::TRAVERSE_CONTINUE;
            
            current = next;
        }
    }
    return Compound::TRAVERSE_CONTINUE;
}

Compound::VisitorResult Compound::accept( ConstCompoundVisitor* visitor,
                                          const bool activeOnly ) const
{
    return _accept( this, visitor, activeOnly );
}

Compound::VisitorResult Compound::accept( CompoundVisitor* visitor,
                                          const bool activeOnly )
{
    return _accept( this, visitor, activeOnly );
}

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::init()
{
    CompoundInitVisitor initVisitor;
    accept( &initVisitor, false /* activeOnly */ );

    // CompoundUpdateVisitor updateVisitor;
}

void Compound::exit()
{
    CompoundExitVisitor visitor;
    accept( &visitor, false /* activeOnly */ );
}

//---------------------------------------------------------------------------
// pre-render compound state update
//---------------------------------------------------------------------------
void Compound::update( const uint32_t frameNumber )
{
    CompoundUpdateDataVisitor updateDataVisitor( frameNumber );
    accept( &updateDataVisitor, false /*activeOnly*/ );

    CompoundUpdateOutputVisitor updateOutputVisitor( frameNumber );
    accept( &updateOutputVisitor );

    const hash_map<std::string, Frame*>& outputFrames =
        updateOutputVisitor.getOutputFrames();
    CompoundUpdateInputVisitor updateInputVisitor( outputFrames );
    accept( &updateInputVisitor );

    const hash_map<std::string, eqNet::Barrier*>& swapBarriers = 
        updateOutputVisitor.getSwapBarriers();

    for( hash_map<string, eqNet::Barrier*>::const_iterator i = 
             swapBarriers.begin(); i != swapBarriers.end(); ++i )
    {
        eqNet::Barrier* barrier = i->second;
        if( barrier->getHeight() > 1 )
            barrier->commit();
    }
}

void Compound::updateInheritData( const uint32_t frameNumber )
{
    _data.pixel.validate();

    if( !_parent )
    {
        _inherit = _data;

        if( _inherit.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP_BIT;

        if( _inherit.period == EQ_UNDEFINED_UINT32 )
            _inherit.period = 1;

        if( _inherit.phase == EQ_UNDEFINED_UINT32 )
            _inherit.phase = 0;

        if( _inherit.channel )
        {
            _inherit.pvp  = _inherit.channel->getPixelViewport();
            _inherit.pvp.apply( _data.vp );
            _inherit.pvp.apply( _data.pixel );
        }

        if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = eq::Frame::BUFFER_COLOR;

        if( _inherit.iAttributes[IATTR_STEREO_MODE] == eq::UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] = eq::QUAD;

        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] == 
            eq::UNDEFINED )

            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                COLOR_MASK_RED;

        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] == 
            eq::UNDEFINED )
            
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] =
                COLOR_MASK_GREEN | COLOR_MASK_BLUE;

        if( _inherit.iAttributes[IATTR_UPDATE_FOV] == eq::UNDEFINED )
            _inherit.iAttributes[IATTR_UPDATE_FOV] = eq::HORIZONTAL;
    }
    else
    {
        _inherit = _parent->_inherit;

        if( !_inherit.channel )
            _inherit.channel = _data.channel;
    
        if( _data.view.isValid( ))
            _inherit.view = _data.view;
        
        _inherit.vp.apply( _data.vp );
        _inherit.range.apply( _data.range );
        _inherit.pixel.apply( _data.pixel );

        if( _data.eyes != EYE_UNDEFINED )
            _inherit.eyes = _data.eyes;
        
        if( _data.period != EQ_UNDEFINED_UINT32 )
            _inherit.period = _data.period;

        if( _data.phase != EQ_UNDEFINED_UINT32 )
            _inherit.phase = _data.phase;

        if ( !_inherit.pvp.isValid() && _inherit.channel )
            _inherit.pvp = _inherit.channel->getPixelViewport();

        if( _inherit.pvp.isValid( ))
        {
            _inherit.pvp.apply( _data.vp );
            _inherit.pvp.apply( _data.pixel );
        }

        if( _data.buffers != eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = _data.buffers;
        
        if( _data.iAttributes[IATTR_STEREO_MODE] != eq::UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] =
                _data.iAttributes[IATTR_STEREO_MODE];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] != eq::UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] !=eq::UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK];

        if( _data.iAttributes[IATTR_UPDATE_FOV] != eq::UNDEFINED )
            _inherit.iAttributes[IATTR_UPDATE_FOV] =
                _data.iAttributes[IATTR_UPDATE_FOV];
    }

    if( _data.tasks == TASK_DEFAULT )
    {
        if( isLeaf( ))
            _inherit.tasks = TASK_ALL;
        else
            _inherit.tasks = TASK_ASSEMBLE | TASK_READBACK;
    }
    else
        _inherit.tasks = _data.tasks;

    _inherit.active = (( frameNumber % _inherit.period ) == _inherit.phase);
}

std::ostream& operator << (std::ostream& os, const Compound* compound)
{
    if( !compound )
        return os;
    
    os << disableFlush << "compound" << endl;
    os << "{" << endl << indent;
      
    const std::string& name = compound->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    const Channel* channel = compound->getChannel();
    if( channel )
    {
        Compound* parent = compound->getParent();
        if( !parent || parent->getChannel() != channel )
        {
            const std::string& channelName = channel->getName();
            if( channelName.empty( ))
                os << "channel  \"channel_" << (void*)channel << "\"" << endl;
            else
                os << "channel  \"" << channelName << "\"" << endl;
        }
    }

    const uint32_t tasks = compound->getTasks();
    if( tasks != Compound::TASK_DEFAULT )
    {
        os << "task     [";
        if( tasks &  Compound::TASK_CLEAR )    os << " CLEAR";
        if( tasks &  Compound::TASK_CULL )     os << " CULL";
        if( compound->isLeaf() && 
            ( tasks &  Compound::TASK_DRAW ))  os << " DRAW";
        if( tasks &  Compound::TASK_ASSEMBLE ) os << " ASSEMBLE";
        if( tasks &  Compound::TASK_READBACK ) os << " READBACK";
        os << " ]" << endl;
    }

    const uint32_t buffers = compound->getBuffers();
    if( buffers != eq::Frame::BUFFER_UNDEFINED )
    {
        os << "buffers  [";
        if( buffers & eq::Frame::BUFFER_COLOR )  os << " COLOR";
        if( buffers & eq::Frame::BUFFER_DEPTH )  os << " DEPTH";
        os << " ]" << endl;
    }

    const eq::Viewport& vp = compound->getViewport();
    if( vp.isValid() && !vp.isFullScreen( ))
        os << "viewport " << vp << endl;
    
    const eq::Range& range = compound->getRange();
    if( range.isValid() && range != eq::Range::ALL )
        os << range << endl;

    const eq::Pixel& pixel = compound->getPixel();
    if( pixel.isValid() && pixel != eq::Pixel::ALL )
        os << pixel << endl;

    const uint32_t eye = compound->getEyes();
    if( eye )
    {
        os << "eye      [ ";
        if( eye & Compound::EYE_CYCLOP_BIT )
            os << "CYCLOP ";
        if( eye & Compound::EYE_LEFT_BIT )
            os << "LEFT ";
        if( eye & Compound::EYE_RIGHT_BIT )
            os << "RIGHT ";
        os << "]" << endl;
    }

    const uint32_t period = compound->getPeriod();
    if( period != EQ_UNDEFINED_UINT32 )
        os << "period   " << period << endl;

    const uint32_t phase = compound->getPhase();
    if( phase != EQ_UNDEFINED_UINT32 )
        os << "phase    " << phase << endl;

    // attributes
    bool attrPrinted = false;
    
    for( Compound::IAttribute i = static_cast< Compound::IAttribute >( 0 );
         i<Compound::IATTR_ALL; 
         i = static_cast< Compound::IAttribute >( static_cast<uint32_t>( i )+1))
    {
        const int value = compound->getIAttribute( i );
        if( value == Global::instance()->getCompoundIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
            attrPrinted = true;
        }
        
        os << ( i==Compound::IATTR_STEREO_MODE ?
                    "stereo_mode                " :
                i==Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK ?
                    "stereo_anaglyph_left_mask  " :
                i==Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK ?
                    "stereo_anaglyph_right_mask " : "ERROR" );
        
        switch( i )
        {
            case Compound::IATTR_STEREO_MODE:
                os << static_cast<eq::IAttrValue>( value ) << endl;
                break;

            case Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK:
            case Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK:
                os << ColorMask( value ) << endl;
                break;

            default:
                EQASSERTINFO( 0, "unimplemented" );
        }
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    switch( compound->getLatestView( ))
    {
        case Compound::VIEW_WALL:
            os << compound->getWall() << endl;
            break;
        case Compound::VIEW_PROJECTION:
            os << compound->getProjection() << endl;
            break;
        default: 
            break;
    }

    const uint32_t nChildren = compound->nChildren();
    if( nChildren > 0 )
    {
        os << endl;
        for( uint32_t i=0; i<nChildren; i++ )
            os << compound->getChild(i);
    }

    const vector<Frame*>& inputFrames = compound->getInputFrames();
    for( vector<Frame*>::const_iterator iter = inputFrames.begin();
         iter != inputFrames.end(); ++iter )
        
        os << "input" << *iter;

    const vector<Frame*>& outputFrames = compound->getOutputFrames();
    for( vector<Frame*>::const_iterator iter = outputFrames.begin();
         iter != outputFrames.end(); ++iter )
        
        os << "output"  << *iter;

    os << compound->getSwapBarrier();

    os << exdent << "}" << endl << enableFlush;
    return os;
}
}
