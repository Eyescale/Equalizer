
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"
#include "colorMask.h"
#include "config.h"
#include "log.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "swapBarrier.h"

#include <eq/base/base.h>
#include <eq/base/stdExt.h>
#include <eq/client/colorMask.h>
#include <eq/client/packets.h>
#include <eq/client/wall.h>
#include <eq/client/windowSystem.h>
#include <eq/net/session.h>

#include <algorithm>
#include <math.h>
#include <vector>

using namespace eqs;
using namespace eqBase;
using namespace std;
using namespace stde;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_COMPOUND_") + #attr )
std::string Compound::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_STEREO_MODE ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_LEFT_MASK ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_RIGHT_MASK )
};

Compound::Compound()
        : _config( 0 ),
          _parent( 0 ),
          _frame( 0 ),
          _swapBarrier( 0 )
{
}

// copy constructor
Compound::Compound( const Compound& from )
        : _config( 0 ),
          _parent( 0 ),
          _frame( 0 )
{
    _name        = from._name;
    _view        = from._view;
    _data        = from._data;
    _swapBarrier = from._swapBarrier;

    const uint32_t nChildren = from.nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
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

Compound* Compound::_getNext() const
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

Channel* Compound::getChannel() const
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return 0;
}

eqs::Window* Compound::getWindow() const
{
    Channel* channel = getChannel();
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
void Compound::setWall( const eq::Wall& wall )
{
    _data.view.applyWall( wall );
    _view.wall   = wall;
    _view.latest = View::WALL;

    EQVERB << "Wall matrix: " << _data.view.xfm << endl;
}

void Compound::setProjection( const eq::Projection& projection )
{
    _data.view.applyProjection( projection );
    _view.projection = projection;
    _view.latest     = View::PROJECTION;
}

void Compound::setView( const eq::View& view )
{
    _data.view   = view;
    _view.view   = view;
    _view.latest = View::VIEW;
}

//---------------------------------------------------------------------------
// traverse
//---------------------------------------------------------------------------
TraverseResult Compound::traverse( Compound* compound, TraverseCB preCB,
                                   TraverseCB leafCB, TraverseCB postCB,
                                   void *userData )
{
    if ( compound->isLeaf( )) 
    {
        if ( leafCB ) 
            return leafCB( compound, userData );
        return TRAVERSE_CONTINUE;
    }

    Compound *current = compound;
    while( true )
    {
        Compound *parent = current->getParent();
        Compound *next   = current->_getNext();
        Compound *child  = (current->nChildren()) ? current->getChild(0) : 0;

        //---------- down-right traversal
        if ( !child ) // leaf
        {
            if ( leafCB )
            {
                TraverseResult result = leafCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }

            current = next;
        } 
        else // node
        {
            if( preCB )
            {
                TraverseResult result = preCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }

            current = child;
        }

        //---------- up-right traversal
        if( !current && !parent ) return TRAVERSE_CONTINUE;

        while( !current )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->_getNext();

            if( postCB )
            {
                TraverseResult result = postCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }
            
            if ( current == compound ) return TRAVERSE_CONTINUE;
            
            current = next;
        }
    }
    return TRAVERSE_CONTINUE;
}

TraverseResult Compound::traverseActive( Compound* compound, TraverseCB preCB,
                                         TraverseCB leafCB, TraverseCB postCB,
                                         void *userData )
{
    if ( compound->isLeaf( )) 
    {
        if ( leafCB && compound->_isActive( )) 
            return leafCB( compound, userData );
        return TRAVERSE_CONTINUE;
    }

    Compound *current = compound;
    while( true )
    {
        Compound *parent = current->getParent();
        Compound *next   = current->_getNext();
        Compound *child  = (current->nChildren()) ? current->getChild(0) : 0;

        //---------- down-right traversal
        if ( !child ) // leaf
        {
            if ( leafCB && current->_isActive( ))
            {
                TraverseResult result = leafCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }

            current = next;
        } 
        else // node
        {
            if( preCB && current->_isActive( ))
            {
                TraverseResult result = preCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }

            if( current->_isActive( ))
                current = child;
            else
                current = next;
        }

        //---------- up-right traversal
        if( !current && !parent ) return TRAVERSE_CONTINUE;

        while( !current )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->_getNext();

            if( postCB && current->_isActive( ))
            {
                TraverseResult result = postCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
            }
            
            if ( current == compound ) return TRAVERSE_CONTINUE;
            
            current = next;
        }
    }
    return TRAVERSE_CONTINUE;
}

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::init()
{
    traverse( this, _initCB, _initCB, 0, 0 );
}

TraverseResult Compound::_initCB( Compound* compound, void* userData )
{
    Channel* channel = compound->getChannel();
    if( channel )
        channel->refUsed();

    Config*             config  = compound->getConfig();
    const uint32_t      latency = config->getLatency();
    EQASSERT( config );
    
    for( vector<Frame*>::iterator iter = compound->_outputFrames.begin(); 
         iter != compound->_outputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        config->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) 
            << "Output frame \"" << frame->getName() << "\" id " 
            << frame->getID() << endl;
    }

    for( vector<Frame*>::const_iterator iter = compound->_inputFrames.begin(); 
         iter != compound->_inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        config->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) 
            << "Input frame \"" << frame->getName() << "\" id " 
            << frame->getID() << endl;
    }
    
    return TRAVERSE_CONTINUE;    
}

void Compound::exit()
{
    Config* config = getConfig();
    EQASSERT( config );

    for( vector<Frame*>::iterator iter = _outputFrames.begin(); 
         iter != _outputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        frame->flush();
        config->deregisterObject( frame );
    }
    _outputFrames.clear();

    for( vector<Frame*>::const_iterator iter = _inputFrames.begin(); 
         iter != _inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        config->deregisterObject( frame );
    }
    _inputFrames.clear();

    const uint32_t nChildren = this->nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
    {
        Compound* child = getChild(i);
        child->exit();
    }

    Channel* channel = getChannel();
    if( channel )
        channel->unrefUsed();
}

//---------------------------------------------------------------------------
// pre-render compound state update
//---------------------------------------------------------------------------
void Compound::update( const uint32_t frameNumber )
{
    _frame = frameNumber;

    UpdateData data;

    traverse(       this, _updateDataCB, _updateDataCB, 0,     0     );
    traverseActive( this, 0, _updateOutputCB, _updateOutputCB, &data );
    traverseActive( this, _updateInputCB, _updateInputCB, 0,   &data );
    
    for( hash_map<string, eqNet::Barrier*>::const_iterator i = 
             data.swapBarriers.begin(); i != data.swapBarriers.end(); ++i )
    {
        eqNet::Barrier* barrier = i->second;
        if( barrier->getHeight() > 1 )
            barrier->commit();
    }
}

TraverseResult Compound::_updateDataCB( Compound* compound, void* userData )
{
    compound->_updateInheritData();
    
    return TRAVERSE_CONTINUE;
}
TraverseResult Compound::_updateOutputCB( Compound* compound, void* userData )
{
    UpdateData* data = (UpdateData*)userData;
    compound->_updateOutput( data );
    compound->_updateSwapBarriers( data );
    
    return TRAVERSE_CONTINUE;
}

TraverseResult Compound::_updateInputCB( Compound* compound, void* userData )
{
    UpdateData* data = (UpdateData*)userData;
    compound->_updateInput( data );
    return TRAVERSE_CONTINUE;
}

void Compound::_updateInheritData()
{
    if( !_parent )
    {
        _inherit = _data;

        if( _inherit.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP;

        if( _inherit.period == EQ_UNDEFINED_UINT32 )
            _inherit.period = 1;

        if( _inherit.phase == EQ_UNDEFINED_UINT32 )
            _inherit.phase = 0;

        if( _inherit.channel )
        {
            _inherit.pvp  = _inherit.channel->getPixelViewport();
            _inherit.pvp *= _data.vp;
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
    }
    else
    {
        _frame   = _parent->_frame;
        _inherit = _parent->_inherit;

        if( !_inherit.channel )
            _inherit.channel = _data.channel;
    
        if( !_inherit.view.isValid( ))
            _inherit.view = _data.view;
        
        _inherit.vp    *= _data.vp;
        _inherit.range *= _data.range;

        if( _data.eyes != EYE_UNDEFINED )
            _inherit.eyes = _data.eyes;
        
        if( _data.period != EQ_UNDEFINED_UINT32 )
            _inherit.period = _data.period;

        if( _data.phase != EQ_UNDEFINED_UINT32 )
            _inherit.phase = _data.phase;

        if ( !_inherit.pvp.isValid() && _inherit.channel )
            _inherit.pvp = _inherit.channel->getPixelViewport();
        if( _inherit.pvp.isValid( ))
            _inherit.pvp *= _data.vp;

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
    }

    if( _data.tasks == TASK_DEFAULT )
    {
        if( _children.empty( ))
            _inherit.tasks = TASK_ALL;
        else
            _inherit.tasks = TASK_ASSEMBLE | TASK_READBACK;
    }
    else
        _inherit.tasks = _data.tasks;
}

void Compound::_updateOutput( UpdateData* data )
{
    const Channel* channel     = getChannel();
    if( !testInheritTask( TASK_READBACK ) || _outputFrames.empty( ) || 
        !channel )

        return;

    for( vector<Frame*>::iterator iter = _outputFrames.begin(); 
         iter != _outputFrames.end(); ++iter )
    {
        Frame*             frame  = *iter;
        const std::string& name   = frame->getName();

        if( data->outputFrames.find( name ) != data->outputFrames.end())
        {
            EQWARN << "Multiple output frames of the same name are unsupported"
                   << ", ignoring output frame " << name << endl;
            frame->unsetData();
            continue;
        }

        const eq::Viewport& frameVP  = frame->getViewport();
        eq::PixelViewport   framePVP = _inherit.pvp * frameVP;

        // FrameData offset is position wrt destination view
        frame->cycleData( _frame );
        FrameData* frameData = frame->getData();
        EQASSERT( frameData );

        frameData->setOffset( vmml::Vector2i( framePVP.x, framePVP.y ));

        EQLOG( eq::LOG_ASSEMBLY )
            << disableFlush << "Output frame \"" << name << "\" id " 
            << frame->getID() << " v" << frame->getVersion()+1
            << " data id " << frameData->getID() << " v" 
            << frameData->getVersion() + 1 << " on channel \""
            << channel->getName() << "\" tile pos " << framePVP.x << ", " 
            << framePVP.y;

        // FrameData pvp is area within channel
        framePVP.x = (int32_t)(frameVP.x * _inherit.pvp.w);
        framePVP.y = (int32_t)(frameVP.y * _inherit.pvp.h);
        frameData->setPixelViewport( framePVP );

        // Frame offset is position wrt window, i.e., the channel position
        if( _inherit.channel == channel
            /* || use dest channel origin hint set */ )

            frame->setOffset( vmml::Vector2i( _inherit.pvp.x, _inherit.pvp.y));
        else
        {
            const eq::PixelViewport& nativePVP = channel->getPixelViewport();
            frame->setOffset( vmml::Vector2i( nativePVP.x, nativePVP.y ));
        }

        // image buffers
        uint32_t buffers = frame->getBuffers();
        frameData->setBuffers( buffers == eq::Frame::BUFFER_UNDEFINED ? 
                               getInheritBuffers() : buffers );

        frameData->commit();
        frame->updateInheritData( this );
        frame->commit();
        data->outputFrames[name] = frame;

        EQLOG( eq::LOG_ASSEMBLY ) 
            << " buffers frame " << frame->getInheritBuffers() << " data " 
            << frameData->getBuffers() << " read area " << framePVP << endl
            << enableFlush;
    }
}

void Compound::_updateSwapBarriers( UpdateData* data )
{
    if( !_swapBarrier )
        return;

    Window* window = getWindow();
    if( !window )
        return;

    const std::string& barrierName = _swapBarrier->getName();
    hash_map<string, eqNet::Barrier*>::iterator iter = 
        data->swapBarriers.find( barrierName );

    if( iter == data->swapBarriers.end( ))
        data->swapBarriers[barrierName] = window->newSwapBarrier();
    else
        window->joinSwapBarrier( iter->second );
}

void Compound::_updateInput( UpdateData* data )
{
    const Channel* channel = getChannel();

    if( !testInheritTask( TASK_ASSEMBLE ) || _inputFrames.empty( ) || !channel )
        return;

    for( vector<Frame*>::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame*                            frame = *i;
        const std::string&                 name = frame->getName();
        hash_map<string, Frame*>::iterator iter = data->outputFrames.find(name);

        if( iter == data->outputFrames.end())
        {
            EQWARN << "Can't find matching output frame, ignoring input frame "
                   << name << endl;
            frame->unsetData();
            continue;
        }

        Frame*              outputFrame = iter->second;
        const eq::Viewport& frameVP     = frame->getViewport();
        eq::PixelViewport   framePVP    = _inherit.pvp * frameVP;
        vmml::Vector2i      frameOffset = outputFrame->getData()->getOffset();

        if( channel != _inherit.channel
            /* && !use dest channel origin hint set */ )
        {
            // compute delta offset between source and destination, since the
            // channel's native origin (as opposed to destination) is used.
            frameOffset.x -= framePVP.x;
            frameOffset.y -= framePVP.y;
        }

        // input frames are moved using the offset. The pvp signifies the pixels
        // to be used from the frame data.
        framePVP.x = static_cast<int32_t>( frameVP.x * _inherit.pvp.w );
        framePVP.y = static_cast<int32_t>( frameVP.y * _inherit.pvp.h );

        frame->setOffset( frameOffset );
        //frame->setPixelViewport( framePVP );
        outputFrame->addInputFrame( frame );
        frame->updateInheritData( this );
        frame->commit();

        EQLOG( eq::LOG_ASSEMBLY )
            << "Input frame  \"" << name << "\" on channel \"" 
            << channel->getName() << "\" id " << frame->getID() << " v"
            << frame->getVersion() << " buffers " << frame->getInheritBuffers() 
            << "\" tile pos " << frameOffset << " sub-pvp " << framePVP << endl;
    }
}

//---------------------------------------------------------------------------
// per-channel update/task generation
//---------------------------------------------------------------------------
void Compound::updateChannel( Channel* channel, const uint32_t frameID )
{
    UpdateChannelData data = { channel, frameID };

    data.eye = EYE_LEFT;
    traverseActive( this, _updatePreDrawCB, _updateDrawCB, _updatePostDrawCB,
                    &data );

    data.eye = EYE_RIGHT;
    traverseActive( this, _updatePreDrawCB, _updateDrawCB, _updatePostDrawCB, 
                    &data );

    data.eye = EYE_CYCLOP;
    traverseActive( this, _updatePreDrawCB, _updateDrawCB, _updatePostDrawCB, 
                    &data );
}

TraverseResult Compound::_updatePreDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->getChannel() != channel ||
        !compound->testInheritTask( TASK_CLEAR ) ||
        !( compound->_inherit.eyes & data->eye ))
        
        return TRAVERSE_CONTINUE;

    // clear task tested above
    eq::ChannelFrameClearPacket clearPacket;        

    compound->_setupRenderContext( clearPacket.context, data );
    channel->send( clearPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK clear " << channel->getName() <<  " "
                           << &clearPacket << endl;
    return TRAVERSE_CONTINUE;
}

// leaf-channel update
TraverseResult Compound::_updateDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->getChannel() != channel || !compound->_inherit.tasks ||
        !(compound->_inherit.eyes & data->eye) )
        
        return TRAVERSE_CONTINUE;

    eq::RenderContext context;
    compound->_setupRenderContext( context, data );
    // OPT: Send render context once before task packets?

    if( compound->testInheritTask( TASK_CLEAR ))
    {
        eq::ChannelFrameClearPacket clearPacket;        
        clearPacket.context = context;
        channel->send( clearPacket );
        EQLOG( eq::LOG_TASKS ) << "TASK clear " << channel->getName() <<  " "
                           << &clearPacket << endl;
    }
    if( compound->testInheritTask( TASK_DRAW ))
    {
        eq::ChannelFrameDrawPacket drawPacket;

        drawPacket.context = context;
        compound->_computeFrustum( drawPacket.context, data->eye );
        channel->send( drawPacket );
        EQLOG( eq::LOG_TASKS ) << "TASK draw " << channel->getName() <<  " " 
                           << &drawPacket << endl;
    }
    
    compound->_updatePostDraw( context );
    return TRAVERSE_CONTINUE;
}

void Compound::_setupRenderContext( eq::RenderContext& context, 
                                    const UpdateChannelData* data )
{
    context.frameID        = data->frameID;
    context.pvp            = _inherit.pvp;
    context.vp             = _inherit.vp;
    context.range          = _inherit.range;
    context.buffer         = _getDrawBuffer( data );
    context.drawBufferMask = _getDrawBufferMask( data );
    const Channel* channel = data->channel;

    if( channel != _inherit.channel /* && !use dest channel origin hint set */ )
    {
        const eq::PixelViewport& nativePVP = channel->getPixelViewport();
        context.pvp.x = nativePVP.x;
        context.pvp.y = nativePVP.y;
    }
    // TODO: pvp size overcommit check?
}

GLenum Compound::_getDrawBuffer( const UpdateChannelData* data )
{
    eq::Window::DrawableConfig drawableConfig = 
        data->channel->getWindow()->getDrawableConfig();
    
    if( !drawableConfig.stereo )
    {    
        if( drawableConfig.doublebuffered )
            return GL_BACK;
        // else singlebuffered
        return GL_FRONT;
    }
    else
    {
        if( drawableConfig.doublebuffered )
        {
            switch( data->eye )
            {
                case EYE_LEFT:
                    return GL_BACK_LEFT;
                    break;
                case EYE_RIGHT:
                    return GL_BACK_RIGHT;
                    break;
                default:
                    return GL_BACK;
                    break;
            }
        }
        // else singlebuffered
        switch( data->eye )
        {
            case EYE_LEFT:
                return GL_FRONT_LEFT;
                break;
            case EYE_RIGHT:
                return GL_FRONT_RIGHT;
                break;
            default:
                return GL_FRONT;
                break;
        }
    }
}

eq::ColorMask Compound::_getDrawBufferMask( const UpdateChannelData* data )
{
    if( _inherit.iAttributes[IATTR_STEREO_MODE] != eq::ANAGLYPH )
        return eq::ColorMask::ALL;

    switch( data->eye )
    {
        case EYE_LEFT:
            return eqs::ColorMask( 
                _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] );
        case EYE_RIGHT:
            return eqs::ColorMask( 
                _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] );
        default:
            return eq::ColorMask::ALL;
    }
}

void Compound::_computeFrustum( eq::RenderContext& context, const Eye whichEye )
{
    const Channel*  destination = _inherit.channel;
    const eq::View& view        = _inherit.view;
    Config*         config      = getConfig();
    destination->getNearFar( &context.frustum.nearPlane, &context.frustum.farPlane );

    // compute eye position in screen space
    const vmml::Vector3f& eyeW = config->getEyePosition( whichEye );
    const vmml::Matrix4f& xfm  = view.xfm;

#if 1
    const float           w    = 
        xfm.ml[3] * eyeW[0] + xfm.ml[7] * eyeW[1] + xfm.ml[11]* eyeW[2] + xfm.ml[15];
    const float  eye[3] = {
        (xfm.ml[0] * eyeW[0] + xfm.ml[4] * eyeW[1] + xfm.ml[8] * eyeW[2] + xfm.ml[12]) / w,
        (xfm.ml[1] * eyeW[0] + xfm.ml[5] * eyeW[1] + xfm.ml[9] * eyeW[2] + xfm.ml[13]) / w,
        (xfm.ml[2] * eyeW[0] + xfm.ml[6] * eyeW[1] + xfm.ml[10]* eyeW[2] + xfm.ml[14]) / w};
#else
    const vmml::Vector3f  eye  = xfm * eyeW;
#endif

    // compute frustum from size and eye position
    vmml::Frustumf& frustum = context.frustum;
    const float     ratio   = frustum.nearPlane / eye[2];
    if( eye[2] > 0 )
    {
        frustum.left   =  ( -view.width*0.5f  - eye[0] ) * ratio;
        frustum.right  =  (  view.width*0.5f  - eye[0] ) * ratio;
        frustum.bottom =  ( -view.height*0.5f - eye[1] ) * ratio;
        frustum.top    =  (  view.height*0.5f - eye[1] ) * ratio;
    }
    else // eye behind near plane - 'mirror' x
    {
        frustum.left   =  (  view.width*0.5f  - eye[0] ) * ratio;
        frustum.right  =  ( -view.width*0.5f  - eye[0] ) * ratio;
        frustum.bottom =  (  view.height*0.5f + eye[1] ) * ratio;
        frustum.top    =  ( -view.height*0.5f + eye[1] ) * ratio;
    }

    // adjust to viewport (screen-space decomposition)
    // Note: may need to be computed in pvp space to avoid rounding problems
    const eq::Viewport vp = _inherit.vp;
    if( !vp.isFullScreen() && vp.isValid( ))
    {
        const float frustumWidth = frustum.right - frustum.left;
        frustum.left  += frustumWidth * vp.x;
        frustum.right  = frustum.left + frustumWidth * vp.w;
        
        const float frustumHeight = frustum.top - frustum.bottom;
        frustum.bottom += frustumHeight * vp.y;
        frustum.top     = frustum.bottom + frustumHeight * vp.h;
    }

    // compute head transform
    // headTransform = -trans(eye) * view matrix (frustum position)
    vmml::Matrix4f& headTransform = context.headTransform;
    for( int i=0; i<16; i += 4 )
    {
        headTransform.ml[i]   = xfm.ml[i]   - eye[0] * xfm.ml[i+3];
        headTransform.ml[i+1] = xfm.ml[i+1] - eye[1] * xfm.ml[i+3];
        headTransform.ml[i+2] = xfm.ml[i+2] - eye[2] * xfm.ml[i+3];
        headTransform.ml[i+3] = xfm.ml[i+3];
    }
}

// non-leaf update, executed after leaf update
TraverseResult Compound::_updatePostDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->getChannel() != channel || !compound->_inherit.tasks ||
        !(compound->_inherit.eyes & data->eye) )

        return TRAVERSE_CONTINUE;

    eq::RenderContext context;
    compound->_setupRenderContext( context, data );
    compound->_updatePostDraw( context );
    return TRAVERSE_CONTINUE;
}

void Compound::_updatePostDraw( const eq::RenderContext& context )
{
    _updateAssemble( context );
    _updateReadback( context );
}

void Compound::_updateAssemble( const eq::RenderContext& context )
{
    if( !testInheritTask( TASK_ASSEMBLE ) || _inputFrames.empty( ))
        return;

    vector<Frame*>               frames;
    vector<eqNet::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator iter = _inputFrames.begin(); 
         iter != _inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;

        if( !frame->hasData( )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( frame );
        frameIDs.push_back( eqNet::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    // assemble task
    Channel*                       channel = getChannel();
    eq::ChannelFrameAssemblePacket packet;
    
    packet.context   = context;
    packet.nFrames   = frames.size();

    EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS ) 
        << "TASK assemble " << channel->getName() <<  " " << &packet << endl;
    channel->send<eqNet::ObjectVersion>( packet, frameIDs );
}
    
void Compound::_updateReadback( const eq::RenderContext& context )
{
    if( !testInheritTask( TASK_READBACK ) || _outputFrames.empty( ))
        return;

    vector<Frame*>               frames;
    vector<eqNet::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator iter = _outputFrames.begin(); 
         iter != _outputFrames.end(); ++iter )
    {
        Frame* frame = *iter;

        if( !frame->hasData( )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( frame );
        frameIDs.push_back( eqNet::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    // readback task
    Channel*                       channel = getChannel();
    eq::ChannelFrameReadbackPacket packet;
    
    packet.context   = context;
    packet.nFrames   = frames.size();

    EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS ) 
        << "TASK readback " << channel->getName() <<  " "
        << &packet << endl;
    channel->send<eqNet::ObjectVersion>( packet, frameIDs );
    
    // transmit tasks
    Node*                 node         = channel->getNode();
    RefPtr<eqNet::Node>   netNode      = node->getNode();
    const eqNet::NodeID&  outputNodeID = netNode->getNodeID();
    for( vector<Frame*>::const_iterator iter = frames.begin();
         iter != frames.end(); ++iter )
    {
        Frame* frame = *iter;

        const vector<Frame*>& inputFrames  = frame->getInputFrames();
        vector<eqNet::NodeID> nodeIDs;
        for( vector<Frame*>::const_iterator iter = inputFrames.begin();
             iter != inputFrames.end(); ++iter )
        {
            const Frame*         frame   = *iter;
            const Node*          node    = frame->getNode();
            RefPtr<eqNet::Node>  netNode = node->getNode();
            const eqNet::NodeID& nodeID  = netNode->getNodeID();
            EQASSERT( node );

            if( nodeID == outputNodeID ) // TODO filter: buffers, vp, eye
                continue;

            nodeIDs.push_back( nodeID );
        }

        // sort & filter dupes
        stde::usort( nodeIDs );

        if( nodeIDs.empty( ))
            continue;

        // send
        eq::ChannelFrameTransmitPacket transmitPacket;
        transmitPacket.sessionID = packet.sessionID;
        transmitPacket.objectID  = packet.objectID;
        transmitPacket.frame     = eqNet::ObjectVersion( frame );
        transmitPacket.nNodes    = nodeIDs.size();

        EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS )
            << "TASK transmit " << channel->getName() <<  " " << 
            &transmitPacket << " first " << nodeIDs[0] << endl;

        channel->send<eqNet::NodeID>( transmitPacket, nodeIDs );
    }        
}

std::ostream& eqs::operator << (std::ostream& os, const Compound* compound)
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
            const std::string& name = channel->getName();
            if( name.empty( ))
                os << "channel  \"channel_" << (void*)channel << "\"" << endl;
            else
                os << "channel  \"" << name << "\"" << endl;
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
    if( range.isValid() && !range.isFull( ))
        os << range << endl;

    const uint32_t eye = compound->getEyes();
    if( eye )
    {
        os << "eye      [ ";
        if( eye & Compound::EYE_CYCLOP )
            os << "CYCLOP ";
        if( eye & Compound::EYE_LEFT )
            os << "LEFT ";
        if( eye & Compound::EYE_RIGHT )
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

    switch( compound->_view.latest )
    {
        case Compound::View::WALL:
            os << compound->getWall() << endl;
            break;
        case Compound::View::PROJECTION:
            //os << compound->getProjection() << endl;
            break;
        case Compound::View::VIEW:
            //os << compound->getView() << endl;
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
