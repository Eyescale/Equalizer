
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"
#include "config.h"
#include "log.h"
#include "frame.h"
#include "frameBuffer.h"
#include "swapBarrier.h"

#include <eq/base/base.h>
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

Compound::Compound()
        : _parent( NULL ),
          _tasks( TASK_ALL ),
          _swapBarrier( NULL )
{
    _data.eyes = EYE_UNDEFINED;
}

// copy constructor
Compound::Compound( const Compound& from )
        : _parent( NULL )
{
    _name        = from._name;
    _tasks       = from._tasks;
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

Compound::InheritData::InheritData()
        : channel( NULL ),
          format( eq::Frame::FORMAT_UNDEFINED )
{}

void Compound::addChild( Compound* child )
{
    _children.push_back( child );
    EQASSERT( !child->_parent );
    child->_parent = this;
}

Compound* Compound::_getNext() const
{
    if( !_parent )
        return NULL;

    vector<Compound*>&          siblings = _parent->_children;
    vector<Compound*>::iterator result   = find( siblings.begin(),
                                                 siblings.end(), this);

    if( result == siblings.end() )
        return NULL;
    result++;
    if( result == siblings.end() )
        return NULL;

    return *result;
}

Channel* Compound::getChannel() const
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return NULL;
}

eqs::Window* Compound::getWindow() const
{
    Channel* channel = getChannel();
    if( channel )
        return channel->getWindow();
    return NULL;
}

void Compound::setSwapBarrier( SwapBarrier* barrier )
{
    if( barrier && barrier->getName().empty( ))
    {
        const Compound* root     = getRoot();
        const string&   rootName = root->getName();
        if( rootName.size() == 0 )
            barrier->setName( "barrier" );
        else
            barrier->setName( "barrier." + rootName );
    }

    _swapBarrier = barrier; 
}

void Compound::addInputFrame( Frame* frame )
{ 
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _inputFrames.push_back( frame ); 
}
void Compound::addOutputFrame( Frame* frame )
{ 
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _outputFrames.push_back( frame ); 
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

    EQVERB << "Wall matrix: " << LOG_MATRIX4x4( _data.view.xfm ) << endl;
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
        Compound *child  = (current->nChildren()) ? current->getChild(0) : NULL;

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
        if( current == NULL && parent == NULL ) return TRAVERSE_CONTINUE;

        while ( current == NULL )
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

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::init()
{
    traverse( this, _initCB, _initCB, NULL, NULL );
}

TraverseResult Compound::_initCB( Compound* compound, void* userData )
{
    Channel* channel = compound->getChannel();
    if( channel )
        channel->refUsed();

    Config*             config  = compound->getConfig();
    RefPtr<eqNet::Node> node    = config->getLocalNode();
    const uint32_t      latency = config->getLatency();
    EQASSERT( config );
    EQASSERT( node );
    
    for( vector<Frame*>::iterator iter = compound->_outputFrames.begin(); 
         iter != compound->_outputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        config->registerObject( frame, node );
        frame->setAutoObsolete( latency );
    }

    for( vector<Frame*>::iterator iter = compound->_inputFrames.begin(); 
         iter != compound->_inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        config->registerObject( frame, node );
        frame->setAutoObsolete( latency );
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

    for( vector<Frame*>::iterator iter = _inputFrames.begin(); 
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
void Compound::update()
{
    UpdateData data;
    traverse( this, _updatePreCB, _updateCB, _updatePostCB, &data );
    traverse( this, _updateInputCB, _updateInputCB, NULL, &data );
    
    for( StringHash<eqNet::Barrier*>::iterator iter = data.swapBarriers.begin();
         iter != data.swapBarriers.end(); ++iter )
 
        iter->second->commit();
}

TraverseResult Compound::_updatePreCB( Compound* compound, void* userData )
{
    compound->_updateInheritData();
    
    return TRAVERSE_CONTINUE;
}
TraverseResult Compound::_updateCB( Compound* compound, void* userData )
{
    UpdateData* data = (UpdateData*)userData;
    compound->_updateInheritData();
    compound->_updateOutput( data );
    compound->_updateSwapBarriers( data );
    
    return TRAVERSE_CONTINUE;
}
TraverseResult Compound::_updatePostCB( Compound* compound, void* userData )
{
    UpdateData* data = (UpdateData*)userData;
    compound->_updateOutput( data );
    compound->_updateSwapBarriers( data );
    
    return TRAVERSE_CONTINUE;
}

void Compound::_updateInheritData()
{
    if( !_parent )
    {
        _inherit = _data;

        if( _data.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP;

        if( _inherit.channel )
            _inherit.pvp = _inherit.channel->getPixelViewport();

        if( _data.format == eq::Frame::FORMAT_UNDEFINED )
            _inherit.format = eq::Frame::FORMAT_COLOR;
        return;
    }

    _inherit = _parent->_inherit;

    if( !_inherit.channel )
        _inherit.channel = _data.channel;

    if( !_inherit.view.isValid( ))
        _inherit.view = _data.view;

    _inherit.vp    *= _data.vp;
    _inherit.range *= _data.range;

    if( _inherit.eyes != EYE_UNDEFINED )
        _inherit.eyes = _data.eyes;
   
    if( _inherit.pvp.isValid( ))
        _inherit.pvp *= _data.vp;
    else if ( _inherit.channel )
        _inherit.pvp = _inherit.channel->getPixelViewport();

    if( _data.format != eq::Frame::FORMAT_UNDEFINED )
        _inherit.format = _data.format;
}

void Compound::_updateOutput( UpdateData* data )
{
    if( !testTask( TASK_READBACK ) || _outputFrames.empty( ) || !_data.channel )
        return;

    const Config*  config      = getConfig();
    const uint32_t frameNumber = config->getFrameNumber();
    const uint32_t latency     = config->getLatency();

    for( vector<Frame*>::iterator iter = _outputFrames.begin(); 
         iter != _outputFrames.end(); ++iter )
    {
        Frame*             frame  = *iter;
        const std::string& name   = frame->getName();

        if( data->outputFrames.find( name ) != data->outputFrames.end())
        {
            EQWARN << "Multiple output frames of the same name are unsupported"
                   << ", ignoring output frame " << name << endl;
            frame->unsetBuffer();
            continue;
        }

        const eq::Viewport& frameVP  = frame->getViewport();
        eq::PixelViewport   framePVP = _inherit.pvp * frameVP;

        // Buffer offset is position of buffer wrt destination view
        frame->cycleBuffer( frameNumber, latency );
        FrameBuffer* buffer = frame->getBuffer();
        EQASSERT( buffer );

        buffer->setOffset( vmml::Vector2i( framePVP.x, framePVP.y ));

        EQLOG( LOG_ASSEMBLY )
            << disableFlush << "Output frame \"" << name << "\" on channel \"" 
            << _data.channel->getName() << "\" tile pos " << framePVP.x << ", "
            << framePVP.y;

        // Buffer pvp is area within channel
        framePVP.x = (int32_t)(frameVP.x * _inherit.pvp.w);
        framePVP.y = (int32_t)(frameVP.y * _inherit.pvp.h);
        buffer->setPixelViewport( framePVP );

        // Frame offset is position wrt window, i.e., the channel position
        if( false /* use dest channel origin hint set */ )
            frame->setOffset( vmml::Vector2i( _inherit.pvp.x, _inherit.pvp.y));
        else
        {
            const eq::PixelViewport& nativePVP =
                _data.channel->getPixelViewport();
            frame->setOffset( vmml::Vector2i( nativePVP.x, nativePVP.y ));
        }

        frame->updateInheritData( this );
        frame->commit();
        data->outputFrames[name] = frame;

        EQLOG( LOG_ASSEMBLY ) 
            << " read area " << framePVP << endl << enableFlush;
    }
}

void Compound::_updateSwapBarriers( UpdateData* data )
{
    Window* window = getWindow();
    if( !window )
        return;

    if( !_swapBarrier )
        return;

    const std::string& barrierName = _swapBarrier->getName();
    StringHash<eqNet::Barrier*>::iterator iter = 
        data->swapBarriers.find( barrierName );

    if( iter == data->swapBarriers.end( ))
        data->swapBarriers[barrierName] = window->newSwapBarrier();
    else
        window->joinSwapBarrier( iter->second );
}

TraverseResult Compound::_updateInputCB( Compound* compound, void* userData )
{
    UpdateData* data = (UpdateData*)userData;
    compound->_updateInput( data );
    return TRAVERSE_CONTINUE;
}

void Compound::_updateInput( UpdateData* data )
{
    if( !testTask( TASK_ASSEMBLE ) || _inputFrames.empty( ) || !_data.channel )
        return;

    for( vector<Frame*>::iterator iter = _inputFrames.begin(); 
         iter != _inputFrames.end(); ++iter )
    {
        Frame*                       frame = *iter;
        const std::string&           name  = frame->getName();
        StringHash<Frame*>::iterator iter  = data->outputFrames.find( name );

        if( iter == data->outputFrames.end())
        {
            EQWARN << "Can't find matching output frame, ignoring input frame "
                   << name << endl;
            frame->unsetBuffer();
            continue;
        }

        const Frame*        outputFrame = iter->second;
        const eq::Viewport& frameVP     = frame->getViewport();
        eq::PixelViewport   framePVP    = _inherit.pvp * frameVP;
        vmml::Vector2i      frameOffset = outputFrame->getOffset();
 
        if( true /* !use dest channel origin hint set */ )
        {
            // compute delta offset between source and destination, since the
            // channel's native origin (as opposed to destination) is used.
            frameOffset.x -= framePVP.x;
            frameOffset.y -= framePVP.y;
        }

        // input frames are moved using the offset. The pvp signifies the pixels
        // to be used from the frame buffer.
        framePVP.x = (int32_t)(frameVP.x * _inherit.pvp.w);
        framePVP.y = (int32_t)(frameVP.y * _inherit.pvp.h);

        frame->setOffset( frameOffset );
        //frame->setPixelViewport( framePVP );
        frame->setOutputFrame( iter->second );
        frame->updateInheritData( this );
        frame->commit();

        EQLOG( LOG_ASSEMBLY )
            << "Input frame \"" << name << "\" on channel \"" 
            << _data.channel->getName() << "\" tile pos " << frameOffset
            << " use pvp from input " << framePVP << endl;
    }
}

//---------------------------------------------------------------------------
// per-channel update/task generation
//---------------------------------------------------------------------------
void Compound::updateChannel( Channel* channel, const uint32_t frameID )
{
    UpdateChannelData data = { channel, frameID };

    data.eye = EYE_LEFT;
    traverse( this, NULL, _updateDrawCB, _updatePostDrawCB, &data );

    data.eye = EYE_RIGHT;
    traverse( this, NULL, _updateDrawCB, _updatePostDrawCB, &data );

    data.eye = EYE_CYCLOP;
    traverse( this, NULL, _updateDrawCB, _updatePostDrawCB, &data );
}

// leaf-channel update
TraverseResult Compound::_updateDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->getChannel() != channel || !compound->_tasks ||
        !(compound->_inherit.eyes & data->eye) )
        
        return TRAVERSE_CONTINUE;

    eq::RenderContext context;
    compound->_setupRenderContext( context, data );

    if( compound->testTask( TASK_CLEAR ))
    {
        eq::ChannelClearPacket clearPacket;        
        clearPacket.context = context;
        channel->send( clearPacket );
        EQLOG( LOG_TASKS ) << "TASK clear " << &clearPacket << endl;
    }
    if( compound->testTask( TASK_DRAW ))
    {
        eq::ChannelDrawPacket drawPacket;

        drawPacket.context = context;
        compound->_computeFrustum( drawPacket.context.frustum, 
                     drawPacket.context.headTransform, data->eye );
        channel->send( drawPacket );
        EQLOG( LOG_TASKS ) << "TASK draw  " << &drawPacket << endl;
    }
    
    compound->_updatePostDraw( context );
    return TRAVERSE_CONTINUE;
}

void Compound::_setupRenderContext( eq::RenderContext& context, 
                                    const UpdateChannelData* data )
{
    context.frameID    = data->frameID;
    context.pvp        = _inherit.pvp;
    context.range      = _inherit.range;

    const Channel* channel = data->channel;
    context.drawBuffer = _getDrawBuffer( data );

    if( true /* !use dest channel origin hint set */ )
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
        else
            return GL_FRONT;
    }
    else
    {
        if( drawableConfig.doublebuffered )
        {
            switch( data->eye )
            {
                case Compound::EYE_LEFT:
                    return GL_BACK_LEFT;
                    break;
                case Compound::EYE_RIGHT:
                    return GL_BACK_RIGHT;
                    break;
                default:
                    return GL_BACK;
                    break;
            }
        }
        else
        {
            switch( data->eye )
            {
                case Compound::EYE_LEFT:
                    return GL_FRONT_LEFT;
                    break;
                case Compound::EYE_RIGHT:
                    return GL_FRONT_RIGHT;
                    break;
                default:
                    return GL_FRONT;
                    break;
            }
        }
    }
}

void Compound::_computeFrustum( eq::Frustum& frustum, float headTransform[16],
                                const Eye whichEye )
{
    const Channel*  destination = _inherit.channel;
    const eq::View& view       = _inherit.view;
    Config*         config      = getConfig();
    destination->getNearFar( &frustum.near, &frustum.far );

    // compute eye position in screen space
    const float* eyeW   = config->getEyePosition( whichEye );
    const float* xfm    = view.xfm;
    const float  w      = 
        xfm[3] * eyeW[0] + xfm[7] * eyeW[1] + xfm[11]* eyeW[2] + xfm[15];
    const float  eye[3] = {
        (xfm[0] * eyeW[0] + xfm[4] * eyeW[1] + xfm[8] * eyeW[2] + xfm[12]) / w,
        (xfm[1] * eyeW[0] + xfm[5] * eyeW[1] + xfm[9] * eyeW[2] + xfm[13]) / w,
        (xfm[2] * eyeW[0] + xfm[6] * eyeW[1] + xfm[10]* eyeW[2] + xfm[14]) / w};

    // compute frustum from size and eye position
    const float ratio = frustum.near / eye[2];
    if( eye[2] > 0 )
    {
        frustum.left   =  ( -view.width/2.  - eye[0] ) * ratio;
        frustum.right  =  (  view.width/2.  - eye[0] ) * ratio;
        frustum.top    =  ( -view.height/2. - eye[1] ) * ratio;
        frustum.bottom =  (  view.height/2. - eye[1] ) * ratio;
    }
    else // eye behind near plane - 'mirror' x
    {
        frustum.left   =  (  view.width/2.  - eye[0] ) * ratio;
        frustum.right  =  ( -view.width/2.  - eye[0] ) * ratio;
        frustum.top    =  (  view.height/2. + eye[1] ) * ratio;
        frustum.bottom =  ( -view.height/2. + eye[1] ) * ratio;
    }

    // adjust to viewport (screen-space decomposition)
    // Note: may need to be computed in pvp space to avoid rounding problems
    const eq::Viewport vp = _inherit.vp;
    if( !vp.isFullScreen() && vp.isValid( ))
    {
        const float frustumWidth = frustum.right - frustum.left;
        frustum.left += frustumWidth * vp.x;
        frustum.right = frustum.left + frustumWidth * vp.w;
        
        const float frustumHeight = frustum.bottom - frustum.top;
        frustum.top   += frustumHeight * vp.y;
        frustum.bottom = frustum.top + frustumHeight * vp.h;
    }

    // compute head transform
    // headTransform = -trans(eye) * view matrix (frustum position)
    for( int i=0; i<16; i += 4 )
    {
        headTransform[i]   = xfm[i]   - eye[0] * xfm[i+3];
        headTransform[i+1] = xfm[i+1] - eye[1] * xfm[i+3];
        headTransform[i+2] = xfm[i+2] - eye[2] * xfm[i+3];
        headTransform[i+3] = xfm[i+3];
    }
}

// non-leaf update, executed after leaf update
TraverseResult Compound::_updatePostDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->getChannel() != channel || !compound->_tasks ||
        !(compound->_inherit.eyes & data->eye) )

        return TRAVERSE_CONTINUE;

    eq::RenderContext context;
    compound->_setupRenderContext( context, data );
    compound->_updatePostDraw( context );
    return TRAVERSE_CONTINUE;
}

void Compound::_updatePostDraw( eq::RenderContext& context )
{
    if( testTask( TASK_READBACK ) && !_outputFrames.empty( ))
    {
        vector<eqNet::ObjectVersion> frames;
        for( vector<Frame*>::iterator iter = _outputFrames.begin(); 
             iter != _outputFrames.end(); ++iter )
        {
            Frame* frame = *iter;
            // TODO: filter
            frames.push_back( eqNet::ObjectVersion( frame ));
        }

        if( !frames.empty() )
        {
            Channel*                  channel = _inherit.channel;
            Node*                     node    = channel->getNode();
            RefPtr<eqNet::Node>       netNode = node->getNode();
            eq::ChannelReadbackPacket packet;
            
            packet.sessionID = channel->getSession()->getID();
            packet.objectID  = channel->getID();;
            packet.context   = context;
            packet.nFrames   = frames.size();
            
            netNode->send<eqNet::ObjectVersion>( packet, frames );
        }
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

    if( !compound->testTask( Compound::TASK_CLEAR ) ||
        !compound->testTask( Compound::TASK_CULL ) ||
        (compound->isLeaf() && !compound->testTask( Compound::TASK_DRAW )) ||
        !compound->testTask( Compound::TASK_ASSEMBLE ) ||
        !compound->testTask( Compound::TASK_READBACK ))
    {
        os << "task     [";
        if( compound->testTask( Compound::TASK_CLEAR ))    os << " CLEAR";
        if( compound->testTask( Compound::TASK_CULL ))     os << " CULL";
        if( compound->isLeaf() && compound->testTask( Compound::TASK_DRAW ))
            os << " DRAW";
        if( compound->testTask( Compound::TASK_ASSEMBLE )) os << " ASSEMBLE";
        if( compound->testTask( Compound::TASK_READBACK )) os << " READBACK";
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
        os << "eye [ ";
        if( eye & Compound::EYE_CYCLOP )
            os << "CYCLOP ";
        if( eye & Compound::EYE_LEFT )
            os << "LEFT ";
        if( eye & Compound::EYE_RIGHT )
            os << "RIGHT ";
        os << "]" << endl;
    }

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

    os << compound->getSwapBarrier();

    const vector<Frame*>& outputFrames = compound->getOutputFrames();
    for( vector<Frame*>::const_iterator iter = outputFrames.begin();
         iter != outputFrames.end(); ++iter )
        
        os << "output"  << *iter << endl;

    const vector<Frame*>& inputFrames = compound->getInputFrames();
    for( vector<Frame*>::const_iterator iter = inputFrames.begin();
         iter != inputFrames.end(); ++iter )
        
        os << "input" << *iter << endl;


    os << exdent << "}" << endl << enableFlush;
    return os;
}
