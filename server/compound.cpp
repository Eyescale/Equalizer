
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"
#include "config.h"
#include "log.h"
#include "frame.h"
#include "frameData.h"
#include "swapBarrier.h"

#include <eq/base/base.h>
#include <eq/base/stdExt.h>
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
        : _config( 0 ),
          _parent( 0 ),
          _tasks( TASK_ALL ),
          _swapBarrier( 0 )
{
}

// copy constructor
Compound::Compound( const Compound& from )
        : _config( 0 ),
          _parent( 0 )
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

Compound::~Compound()
{
    if( _config )
        _config->removeCompound( this );

    _config = 0;

    for( vector<Compound*>::const_iterator i = _children.begin(); 
         i != _children.end(); ++i )
    {
        Compound* compound = *i;

        compound->_parent = NULL;
        delete compound;
    }
    _children.clear();

    for( vector<Frame*>::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = NULL;
        delete frame;
    }
    _inputFrames.clear();

    for( vector<Frame*>::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = NULL;
        delete frame;
    }
    _outputFrames.clear();
}

Compound::InheritData::InheritData()
        : channel( NULL ),
          buffers( eq::Frame::BUFFER_UNDEFINED ),
          eyes( EYE_UNDEFINED )
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

    for( vector<Frame*>::const_iterator iter = compound->_inputFrames.begin(); 
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

        if( _inherit.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP;

        if( _inherit.channel )
        {
            _inherit.pvp  = _inherit.channel->getPixelViewport();
            _inherit.pvp *= _data.vp;
        }

        if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = eq::Frame::BUFFER_COLOR;
        return;
    }

    _inherit = _parent->_inherit;

    if( !_inherit.channel )
        _inherit.channel = _data.channel;

    if( !_inherit.view.isValid( ))
        _inherit.view = _data.view;

    _inherit.vp    *= _data.vp;
    _inherit.range *= _data.range;

    if( _data.eyes != EYE_UNDEFINED )
        _inherit.eyes = _data.eyes;
   
    if ( !_inherit.pvp.isValid() && _inherit.channel )
        _inherit.pvp = _inherit.channel->getPixelViewport();
    if( _inherit.pvp.isValid( ))
        _inherit.pvp *= _data.vp;

    if( _data.buffers != eq::Frame::BUFFER_UNDEFINED )
        _inherit.buffers = _data.buffers;
}

void Compound::_updateOutput( UpdateData* data )
{
    if( !testTask( TASK_READBACK ) || _outputFrames.empty( ) || !_data.channel )
        return;

    const Config*  config      = getConfig();
    const uint32_t frameNumber = config->getFrameNumber();

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

        // Data offset is position of data wrt destination view
        frame->cycleData( frameNumber );
        FrameData* frameData = frame->getData();
        EQASSERT( frameData );

        frameData->setOffset( vmml::Vector2i( framePVP.x, framePVP.y ));

        EQLOG( eq::LOG_ASSEMBLY )
            << disableFlush << "Output frame \"" << name << "\" on channel \"" 
            << _data.channel->getName() << "\" tile pos " << framePVP.x << ", "
            << framePVP.y;

        // FrameData pvp is area within channel
        framePVP.x = (int32_t)(frameVP.x * _inherit.pvp.w);
        framePVP.y = (int32_t)(frameVP.y * _inherit.pvp.h);
        frameData->setPixelViewport( framePVP );

        // Frame offset is position wrt window, i.e., the channel position
        if( _inherit.channel == _data.channel 
            /* || use dest channel origin hint set */ )

            frame->setOffset( vmml::Vector2i( _inherit.pvp.x, _inherit.pvp.y));
        else
        {
            const eq::PixelViewport& nativePVP =
                _data.channel->getPixelViewport();
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
            << " read area " << framePVP << endl << enableFlush;
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

    for( vector<Frame*>::const_iterator iter = _inputFrames.begin(); 
         iter != _inputFrames.end(); ++iter )
    {
        Frame*                       frame = *iter;
        const std::string&           name  = frame->getName();
        StringHash<Frame*>::iterator iter  = data->outputFrames.find( name );

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
        vmml::Vector2i      frameOffset = outputFrame->getOffset();
 
        if( _data.channel != _inherit.channel 
            /* || !use dest channel origin hint set */ )
        {
            // compute delta offset between source and destination, since the
            // channel's native origin (as opposed to destination) is used.
            frameOffset.x -= framePVP.x;
            frameOffset.y -= framePVP.y;
        }

        // input frames are moved using the offset. The pvp signifies the pixels
        // to be used from the frame data.
        framePVP.x = (int32_t)(frameVP.x * _inherit.pvp.w);
        framePVP.y = (int32_t)(frameVP.y * _inherit.pvp.h);

        frame->setOffset( frameOffset );
        //frame->setPixelViewport( framePVP );
        outputFrame->addInputFrame( frame );
        frame->updateInheritData( this );
        frame->commit();

        EQLOG( eq::LOG_ASSEMBLY )
            << "Input frame  \"" << name << "\" on channel \"" 
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
    // OPT: Send render context once before task packets?

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
        compound->_computeFrustum( drawPacket.context, data->eye );
        channel->send( drawPacket );
        EQLOG( LOG_TASKS ) << "TASK draw  " << &drawPacket << endl;
    }
    
    compound->_updatePostDraw( context );
    return TRAVERSE_CONTINUE;
}

void Compound::_setupRenderContext( eq::RenderContext& context, 
                                    const UpdateChannelData* data )
{
    context.frameID        = data->frameID;
    context.pvp            = _inherit.pvp;
    context.range          = _inherit.range;
    context.buffer         = _getGLBuffer( data );
    const Channel* channel = data->channel;

    if( channel != _inherit.channel /* || !use dest channel origin hint set */ )
    {
        const eq::PixelViewport& nativePVP = channel->getPixelViewport();
        context.pvp.x = nativePVP.x;
        context.pvp.y = nativePVP.y;
    }
    // TODO: pvp size overcommit check?
}

GLenum Compound::_getGLBuffer( const UpdateChannelData* data )
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
        // else singlebuffered
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

void Compound::_computeFrustum( eq::RenderContext& context, const Eye whichEye )
{
    const Channel*  destination = _inherit.channel;
    const eq::View& view        = _inherit.view;
    Config*         config      = getConfig();
    destination->getNearFar( &context.frustum.near, &context.frustum.far );

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
    const float     ratio   = frustum.near / eye[2];
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
        
        const float frustumHeight = frustum.bottom -frustum.top;
        frustum.top   += frustumHeight * vp.y;
        frustum.bottom = frustum.top + frustumHeight * vp.h;
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

    if( compound->getChannel() != channel || !compound->_tasks ||
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
    if( !testTask( TASK_ASSEMBLE ) || _inputFrames.empty( ))
        return;

    vector<Frame*>               frames;
    vector<eqNet::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator iter = _inputFrames.begin(); 
         iter != _inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        // TODO: filter: buffers, vp, eye
        frames.push_back( frame );
        frameIDs.push_back( eqNet::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    // assemble task
    Channel*                  channel = getChannel();
    Node*                     node    = channel->getNode();
    RefPtr<eqNet::Node>       netNode = node->getNode();
    eq::ChannelAssemblePacket packet;
    
    packet.sessionID = channel->getSession()->getID();
    packet.objectID  = channel->getID();
    packet.context   = context;
    packet.nFrames   = frames.size();

    EQLOG( eq::LOG_ASSEMBLY | LOG_TASKS ) 
        << "TASK assemble " << &packet << endl;
    netNode->send<eqNet::ObjectVersion>( packet, frameIDs );
}
    
void Compound::_updateReadback( const eq::RenderContext& context )
{
    if( !testTask( TASK_READBACK ) || _outputFrames.empty( ))
        return;

    vector<Frame*>               frames;
    vector<eqNet::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator iter = _outputFrames.begin(); 
         iter != _outputFrames.end(); ++iter )
    {
        Frame* frame = *iter;
        // TODO: filter: buffers, vp, eye
        frames.push_back( frame );
        frameIDs.push_back( eqNet::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    // readback task
    Channel*                  channel = getChannel();
    Node*                     node    = channel->getNode();
    RefPtr<eqNet::Node>       netNode = node->getNode();
    eq::ChannelReadbackPacket packet;
    
    packet.sessionID = channel->getSession()->getID();
    packet.objectID  = channel->getID();
    packet.context   = context;
    packet.nFrames   = frames.size();

    EQLOG( eq::LOG_ASSEMBLY | LOG_TASKS ) 
        << "TASK readback " << &packet << endl;
    netNode->send<eqNet::ObjectVersion>( packet, frameIDs );
    
    // transmit tasks
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
        eq::ChannelTransmitPacket transmitPacket;
        transmitPacket.sessionID = packet.sessionID;
        transmitPacket.objectID  = packet.objectID;
        transmitPacket.frame     = eqNet::ObjectVersion( frame );
        transmitPacket.nNodes    = nodeIDs.size();

        EQLOG( eq::LOG_ASSEMBLY | LOG_TASKS )
            << "TASK transmit " << &transmitPacket << " first " << nodeIDs[0] 
            << endl;

        netNode->send<eqNet::NodeID>( transmitPacket, nodeIDs );
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
