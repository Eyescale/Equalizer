
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "channelStatistics.h"
#include "compositor.h"
#include "commands.h"
#include "configEvent.h"
#include "frame.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "range.h"
#include "renderContext.h"
#include "task.h"
#include "view.h"
#include "frameBufferObject.h"

#include <eq/net/command.h>

using namespace eq::base;
using namespace std;

namespace eq
{
typedef net::CommandFunc<Channel> ChannelFunc;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CHANNEL_") + #attr )
std::string Channel::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
};

Channel::Channel( Window* parent )
        : _window( parent )
        , _tasks( TASK_NONE )
        , _context( 0 )
        , _fixedPVP( false )
        , _frustum( vmml::Frustumf::DEFAULT )
        , _ortho( vmml::Frustumf::DEFAULT )
        , _fbo(0)
        , _drawable( 0 )
        , _view( 0 )
{
    parent->_addChannel( this );
    EQINFO << " New eq::Channel @" << (void*)this << endl;
}

Channel::~Channel()
{  
    EQINFO << " Delete eq::Channel @" << (void*)this << endl;
    _window->_removeChannel( this );
}

void Channel::attachToSession( const uint32_t id, 
                              const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _window->getPipeThreadQueue();

    registerCommand( CMD_CHANNEL_CONFIG_INIT, 
                     ChannelFunc( this, &Channel::_cmdConfigInit ), queue );
    registerCommand( CMD_CHANNEL_CONFIG_EXIT, 
                     ChannelFunc( this, &Channel::_cmdConfigExit ), queue );
    registerCommand( CMD_CHANNEL_FRAME_START,
                     ChannelFunc( this, &Channel::_cmdFrameStart ), queue );
    registerCommand( CMD_CHANNEL_FRAME_FINISH,
                     ChannelFunc( this, &Channel::_cmdFrameFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_CLEAR, 
                     ChannelFunc( this, &Channel::_cmdFrameClear ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW, 
                     ChannelFunc( this, &Channel::_cmdFrameDraw ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW_FINISH, 
                    ChannelFunc( this, &Channel::_cmdFrameDrawFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_ASSEMBLE, 
                     ChannelFunc( this, &Channel::_cmdFrameAssemble ), queue );
    registerCommand( CMD_CHANNEL_FRAME_READBACK, 
                     ChannelFunc( this, &Channel::_cmdFrameReadback ), queue );
    registerCommand( CMD_CHANNEL_FRAME_TRANSMIT, 
                     ChannelFunc( this, &Channel::_cmdFrameTransmit ), queue );
}

bool Channel::configExit()
{
    delete _fbo;
    _fbo = 0;
    return true;
}
bool Channel::configInit( const uint32_t initID )
{ 
    return _configInitFBO(); 
}

bool Channel::_configInitFBO()
{   
    if ( _drawable == FBO_NONE )
        return true;
    
    if  (  !_window->getOSWindow()  ||
          !GLEW_ARB_texture_non_power_of_two ||
          !GLEW_EXT_framebuffer_object )
    {
    	setErrorMessage( "Can't use FBO due to missing GL extensions" );
        return false;
    }
        
    // needs glew initialized (see above)
    _fbo = new FrameBufferObject( glewGetContext() );
        
    if( _fbo->init( _pvp.w, _pvp.h, _drawable & FBO_DEPTH,
                   _drawable & FBO_STENCIL ) ) 
    {
         return true;
    }

    setErrorMessage( "FBO initialization failed" );
    delete _fbo;
    _fbo = 0;
    return false;
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Channel::_setPixelViewport( const PixelViewport& pvp )
{
    EQASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    _fixedPVP = true;

    if( _pvp == pvp && _vp.hasArea( ))
        return;

    _pvp = pvp;
    _vp.invalidate();

    if( !_window )
        return;
    
    const PixelViewport& windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
        _vp = pvp.getSubVP( windowPVP );

    EQVERB << "Channel pvp set: " << _pvp << ":" << _vp << endl;
}

void Channel::_setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _fixedPVP = false;

    if( _vp == vp && _pvp.hasArea( ))
        return;

    _vp = vp;
    _pvp.invalidate();

    if( !_window )
        return;

    PixelViewport windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
    {
        windowPVP.x = 0;
        windowPVP.y = 0;
        _pvp = windowPVP.getSubPVP( vp );

        // send event
        Event event;
        event.type       = Event::CHANNEL_RESIZE;
        event.originator = getID();
        event.resize.x   = _pvp.x;
        event.resize.y   = _pvp.y;
        event.resize.w   = _pvp.w;
        event.resize.h   = _pvp.h;

        processEvent( event );
    }

    EQVERB << "Channel vp set: " << _pvp << ":" << _vp << endl;
}

void Channel::_notifyViewportChanged()
{
    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedPVP ) // update viewport
        _vp = _pvp.getSubVP( windowPVP );
    else            // update pixel viewport
    {
        eq::PixelViewport pvp = windowPVP.getSubPVP( _vp );
        if( _pvp == pvp )
            return;

        _pvp = pvp;

        // send event
        Event event;
        event.type       = Event::CHANNEL_RESIZE;
        event.originator = getID();
        event.resize.x   = _pvp.x;
        event.resize.y   = _pvp.y;
        event.resize.w   = _pvp.w;
        event.resize.h   = _pvp.h;

        processEvent( event );
    }

    EQINFO << "Channel viewport update: " << _pvp << ":" << _vp << endl;
}

void Channel::setNearFar( const float nearPlane, const float farPlane )
{
    _frustum.adjustNear( nearPlane );
    _frustum.farPlane = farPlane;
    _ortho.nearPlane = nearPlane;
    _ortho.farPlane  = farPlane;

    if( _context )
    {
        _context->frustum.adjustNear( nearPlane );
        _context->frustum.farPlane = farPlane;
        _context->ortho.nearPlane = nearPlane;
        _context->ortho.farPlane  = farPlane;
    }

    if( _frustum.nearPlane == nearPlane && _frustum.farPlane == farPlane )
        return;

    ChannelSetNearFarPacket packet;
    packet.nearPlane = nearPlane;
    packet.farPlane  = farPlane;
    
    net::NodePtr node = RefPtr_static_cast< Server, net::Node >( getServer( ));
    send( node, packet );
}

void Channel::addStatistic( Event& event )
{
    _statistics.push_back( event.statistic );
    processEvent( event );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------

void Channel::frameClear( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

#ifndef NDEBUG
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const vmml::Vector3ub color = getUniqueColor();
        glClearColor( color.r/255.0f, color.g/255.0f, color.b/255.0f, 1.0f );
    }
#endif // DEBUG

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    
    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyHeadTransform( ));
}

void Channel::frameAssemble( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Compositor::assembleFrames( getInputFrames(), this );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager* glObjects = getWindow()->getObjectManager();

    const FrameVector& frames = getOutputFrames();
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->startReadback( glObjects );
    }
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->syncReadback();
    }

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::setupAssemblyState()
{
    EQ_GL_ERROR( "before setupAssemblyState" );
    glPushAttrib( GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_VIEWPORT_BIT | 
                  GL_SCISSOR_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT | 
                  GL_POLYGON_BIT );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_3D );
    glDisable( GL_FOG );
    glDisable( GL_CLIP_PLANE0 );
    glDisable( GL_CLIP_PLANE1 );
    glDisable( GL_CLIP_PLANE2 );
    glDisable( GL_CLIP_PLANE3 );
    glDisable( GL_CLIP_PLANE4 );
    glDisable( GL_CLIP_PLANE5 );
    
    glPolygonMode( GL_FRONT, GL_FILL );

    EQASSERT( _window );    
    const PixelViewport& pvp = _window->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    glViewport( 0, 0, pvp.w, pvp.h );
    glScissor( 0, 0, pvp.w, pvp.h );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0f, pvp.w, 0.0f, pvp.h, -1.0f, 1.0f );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    EQ_GL_ERROR( "after  setupAssemblyState" );
}

void Channel::resetAssemblyState()
{
    EQ_GL_ERROR( "before resetAssemblyState" );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glPopAttrib();
    EQ_GL_ERROR( "after  resetAssemblyState" );
}

void Channel::_setRenderContext( RenderContext& context )
{
    _context = &context;
    _window->addRenderContext( context );
}

const Viewport& Channel::getViewport() const
{
    return _context ? _context->vp : _vp;
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _context ? _context->pvp : _pvp;
}

const vmml::Vector2i& Channel::getPixelOffset() const
{
    return _context ? _context->offset : vmml::Vector2i::ZERO;
}

uint32_t Channel::getDrawBuffer() const
{
    return _context ? _context->buffer : GL_BACK;
}

uint32_t Channel::getReadBuffer() const
{
    return _context ? _context->buffer : GL_BACK;
}

const ColorMask& Channel::getDrawBufferMask() const
{
    return _context ? _context->drawBufferMask : ColorMask::ALL;
}

const vmml::Frustumf& Channel::getFrustum() const
{
    return _context ? _context->frustum : _frustum;
}

const vmml::Frustumf& Channel::getOrtho() const
{
    return _context ? _context->ortho : _ortho;
}

const Range& Channel::getRange() const
{
    return _context ? _context->range : Range::ALL;
}

const Pixel& Channel::getPixel() const
{
    return _context ? _context->pixel : Pixel::ALL;
}

const Zoom& Channel::getZoom() const
{
    return _context ? _context->zoom : Zoom::NONE;
}

Eye Channel::getEye() const
{
    return _context ? _context->eye : EYE_CYCLOP;
}

const vmml::Matrix4f& Channel::getHeadTransform() const
{
    return _context ? _context->headTransform : vmml::Matrix4f::IDENTITY;
}

const vmml::Vector2i& Channel::getScreenOrigin() const
{
    return _context ? _context->screenOrigin : vmml::Vector2i::ZERO;
}

vmml::Vector2i Channel::getScreenSize() const
{
    return _context ? _context->screenSize : vmml::Vector2i( _pvp.w, _pvp.h );
}

vmml::Frustumf Channel::getScreenFrustum() const
{
    vmml::Vector2i       origin = getScreenOrigin();
    const PixelViewport& pvp    = getPixelViewport();
    const Pixel&         pixel  = getPixel();

    origin.x += pixel.x;
    origin.y += pixel.y;

    return vmml::Frustumf( origin.x, origin.x + pvp.w * pixel.w,
                           origin.y, origin.y + pvp.h * pixel.h,
                           -1.f, 1.f );
}

FrameBufferObject* Channel::getFrameBufferObject()
{
    return _fbo;
}

void Channel::applyFrameBufferObject()
{
    if( _fbo )
    {
        _fbo->resize( _pvp.w, _pvp.h );
        _fbo->bind(); 
    }
    else if( GLEW_EXT_framebuffer_object )
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void Channel::applyBuffer()
{
    if ( !_fbo )
    {
        EQ_GL_CALL( glReadBuffer( getReadBuffer( )));
        EQ_GL_CALL( glDrawBuffer( getDrawBuffer( )));
    }
    
    applyColorMask();
}

void Channel::bindFramebuffer()
{
   if( !_window->getOSWindow() )
       return;
        
   if( _fbo )
       applyFrameBufferObject();
   else
       _window->bindFramebuffer();
}

void Channel::applyColorMask() const
{
	const ColorMask& colorMask = getDrawBufferMask();
	glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
}

void Channel::applyViewport() const
{
	const PixelViewport& pvp = getPixelViewport();
	// TODO: OPT return if vp unchanged

	if( !pvp.hasArea( ))
	{ 
		EQERROR << "Can't apply viewport " << pvp << endl;
		return;
	}

	EQ_GL_CALL( glViewport( pvp.x, pvp.y, pvp.w, pvp.h ));
    EQ_GL_CALL( glScissor( pvp.x, pvp.y, pvp.w, pvp.h ));
}

void Channel::applyFrustum() const
{
	const vmml::Frustumf& frustum = getFrustum();
	EQ_GL_CALL( glFrustum( frustum.left, frustum.right,             \
                           frustum.bottom, frustum.top,             \
                           frustum.nearPlane, frustum.farPlane )); 
	EQVERB << "Apply " << frustum << endl;
}

void Channel::applyOrtho() const
{
	const vmml::Frustumf& ortho = getOrtho();
	EQ_GL_CALL( glOrtho( ortho.left, ortho.right,               \
                         ortho.bottom, ortho.top,               \
                         ortho.nearPlane, ortho.farPlane )); 
	EQVERB << "Apply " << ortho << endl;
}

void Channel::applyScreenFrustum() const
{
    const vmml::Frustumf frustum = getScreenFrustum();
    EQ_GL_CALL( glOrtho( frustum.left, frustum.right,               \
                         frustum.bottom, frustum.top,               \
                         frustum.nearPlane, frustum.farPlane ));
    EQVERB << "Apply " << frustum << endl;
}

void Channel::applyHeadTransform() const
{
    const vmml::Matrix4f& xfm = getHeadTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.ml ));
    EQVERB << "Apply head transform: " << xfm << endl;
}

bool Channel::processEvent( const Event& event )
{
    ConfigEvent configEvent;
    configEvent.data = event;

    switch( event.type )
    {
        case Event::STATISTIC:
            break;

        case Event::CHANNEL_RESIZE:
        {
            const View* view = getView();
            if( !view )
                return true;

            // transform to view event, which is meaningful for the config 
            configEvent.data.type       = Event::VIEW_RESIZE;
            configEvent.data.originator = view->getID();
            break;
        }

        default:
            EQWARN << "Unhandled channel event of type " << event.type
                   << endl;
            EQUNIMPLEMENTED;
    }

    Config* config = getConfig();
    config->sendEvent( configEvent );
    return true;
}

#define HEIGHT 12
#define SPACE  2

void Channel::drawStatistics()
{
    Config* config = getConfig();
    EQASSERT( config );

    vector< FrameStatistics > statistics;
    config->getStatistics( statistics );

    if( statistics.empty( )) 
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    glClear( GL_DEPTH_BUFFER_BIT );

    glDisable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );

    const util::BitmapFont& font =_window->getObjectManager()->getDefaultFont();

    int64_t       xStart = 0;
    PixelViewport pvp    = _window->getPixelViewport();
    pvp.x = 0;
    pvp.y = 0;

    // find min/max time
    int64_t                 xMax      = 0;
    int64_t                 xMin      = std::numeric_limits< int64_t >::max();

    for( vector< FrameStatistics >::const_iterator i = statistics.begin();
         i != statistics.end(); ++i )
    {
        const FrameStatistics&  frameStats  = *i;
        const SortedStatistics& configStats = frameStats.second;

        for( SortedStatistics::const_iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const Statistics& stats = j->second;

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;
                xMax = EQ_MAX( xMax, stat.endTime );
                xMin = EQ_MIN( xMin, stat.endTime );
            }
        }
    }
    uint32_t scale = 1;
    while( (xMax - xMin) / scale > pvp.w )
        scale *= 10;

    xMax  /= scale;
    xStart = xMax - pvp.getXEnd() + SPACE;
    uint32_t                       nextY = pvp.getYEnd() - SPACE;
    std::map< uint32_t, uint32_t > positions;

    float dim = 0.0f;
    for( vector< FrameStatistics >::reverse_iterator i = statistics.rbegin();
         i != statistics.rend(); ++i )
    {
        const FrameStatistics&  frameStats  = *i;
        const SortedStatistics& configStats = frameStats.second;

        int64_t     frameMin = xMax;
        int64_t     frameMax = 0;

        // draw stats
        for( SortedStatistics::const_iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const uint32_t    id    = j->first;
            const Statistics& stats = j->second;

            if( stats.empty( ))
                continue;

            if( positions.find( id ) == positions.end( ))
            {
                positions.insert( 
                    std::pair< uint32_t, uint32_t >( id, nextY ));
                nextY -= (HEIGHT + SPACE);
            }

            const uint32_t y = positions[ id ];

            const Statistic& nameStat = stats.front();
            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( 100.f, y-SPACE-12.0f, 0.99f );
            font.draw( nameStat.resourceName );

            glBegin( GL_QUADS );
            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;
                const int64_t startTime = stat.startTime / scale;
                const int64_t endTime   = stat.endTime   / scale;

                frameMin = EQ_MIN( frameMin, startTime );
                frameMax = EQ_MAX( frameMax, endTime   );

                if( endTime < xStart || endTime == startTime )
                    continue;

                float y1 = y;
                float y2 = y - HEIGHT;
                float z  = 0.0f;

                switch( stat.type )
                {
                    case Statistic::CHANNEL_CLEAR:
                        glColor3f( .5f-dim, 1.0f-dim, .5f-dim );
                        break;
                    case Statistic::CHANNEL_DRAW:
                        glColor3f( 0.f, 1.0f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_DRAW_FINISH:
                        glColor3f( 0.f, .5f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_ASSEMBLE:
                        glColor3f( 1.0f-dim, 1.0f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_READBACK:
                        glColor3f( 1.0f-dim, .5f-dim, .5f-dim ); 
                        break;
                    case Statistic::NODE_TRANSMIT:
                    case Statistic::CHANNEL_TRANSMIT:
                        glColor3f( 0.f, 0.f, 1.0f-dim ); 
                        z = 0.5f; 
                        break;
                    case Statistic::CHANNEL_TRANSMIT_NODE:
                        glColor3f( 0.5f-dim, 0.5f-dim, 1.0f-dim ); 
                        y1 -= SPACE;
                        z = 0.6f; 
                        break;
                    case Statistic::CHANNEL_COMPRESS:
                    case Statistic::NODE_COMPRESS:
                        glColor3f( 1.0f-dim, 1.0f-dim, 1.0f-dim ); 
                        y1 -= SPACE;
                        z = 0.7f; 
                        break;
                    case Statistic::CHANNEL_WAIT_FRAME:
                    case Statistic::CONFIG_WAIT_FINISH_FRAME:
                        glColor3f( 1.0f-dim, 0.f, 0.f ); 
                        y1 -= SPACE;
                        y2 += SPACE;
                        z = 0.1f; 
                        break;

                    case Statistic::WINDOW_FINISH:
                        glColor3f( 1.0f-dim, .5f-dim, 0.f ); 
                        break;

                    case Statistic::WINDOW_SWAP_BARRIER:
                        glColor3f( 1.0f-dim, 0.f, 0.f ); 
                        break;

                    case Statistic::CONFIG_START_FRAME:
                        glColor3f( .5f-dim, 1.0f-dim, .5f-dim ); 
                        z = 0.1f; 
                        break;
                    case Statistic::CONFIG_FINISH_FRAME:
                        glColor3f( .5f-dim, .5f-dim, .5f-dim ); 
                        break;

                    default:
                        glColor3f( 1.0f-dim, 1.0f-dim, 1.0f-dim ); 
                        z = 0.2f; 
                        break;
                }

                const float x1 = startTime - xStart;
                const float x2 = endTime   - xStart;
                
                glVertex3f( x2, y1, z );
                glVertex3f( x1, y1, z );
                glVertex3f( x1, y2, z );
                glVertex3f( x2, y2, z );
            }
            glEnd();
        }

        frameMin -= xStart;
        frameMax -= xStart;

        glBegin( GL_QUADS );
        glColor3f( .5f-dim, 1.0f-dim, .5f-dim );
        glVertex3f( frameMin+1.0f, pvp.getYEnd(), 0.3f );
        glVertex3f( frameMin,      pvp.getYEnd(), 0.3f );
        glVertex3f( frameMin,      nextY,         0.3f );
        glVertex3f( frameMin+1.0f, nextY,         0.3f );

        glColor3f( .5f-dim, .5f-dim, .5f-dim );
        glVertex3f( frameMax+1.0f, pvp.getYEnd(), 0.3f );
        glVertex3f( frameMax,      pvp.getYEnd(), 0.3f );
        glVertex3f( frameMax,      nextY,         0.3f );
        glVertex3f( frameMax+1.0f, nextY,         0.3f );
        glEnd();

        dim += .2f;
    }

    glColor3f( 1.f, 1.f, 1.f );
    ostringstream scaleText;
    scaleText << ": " << scale << "ms/pixel";
    font.draw( scaleText.str( ));

    _window->drawFPS();
    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::outlineViewport()
{
    setupAssemblyState();

    const PixelViewport& pvp = getPixelViewport();
    glDisable( GL_LIGHTING );
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINE_LOOP );
    {
        glVertex3f( pvp.x + .5f,         pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.getYEnd() - .5f, 0.f );
        glVertex3f( pvp.x + .5f,         pvp.getYEnd() - .5f, 0.f );
    } 
    glEnd();

    resetAssemblyState();
}


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Channel::_cmdConfigInit( net::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK channel config init " << packet << endl;

    if( packet->viewID != EQ_ID_INVALID )
    {
        View*   view   = new View;
        Config* config = getConfig();

        if( config->mapObject( view, packet->viewID ))
            _view = view;
        else
        {
            delete view;
            EQUNREACHABLE;
        }
    }

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );

    _name     = packet->name;
    _tasks    = packet->tasks;
    _color    = packet->color;
    _drawable = packet->drawable;
    
    memcpy( _iAttributes, packet->iAttributes, IATTR_ALL * sizeof( int32_t ));

    _error.clear();
    ChannelConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );

    reply.nearPlane   = _frustum.nearPlane;
    reply.farPlane    = _frustum.farPlane;

    EQLOG( LOG_TASKS ) << "TASK channel config init reply " << &reply << endl;
    send( command.getNode(), reply, _error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdConfigExit( net::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    ChannelConfigExitReplyPacket reply;
    reply.result = configExit();

    if( _view )
    {
        Config* config = getConfig();

        config->unmapObject( _view );
        delete _view;
        _view = 0;
    }

    send( command.getNode(), reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameStart( net::Command& command )
{
    const ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    if( _view )
        _view->sync( packet->viewVersion );
    
    bindFramebuffer();
    frameStart( packet->frameID, packet->frameNumber );

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameFinish( net::Command& command )
{
    const ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet << endl;

    frameFinish( packet->frameID, packet->frameNumber );

    ChannelFrameFinishReplyPacket reply( packet );
    reply.nStatistics = _statistics.size();

    command.getNode()->send( reply, _statistics );

    _statistics.clear();
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameClear( net::Command& command )
{
    ChannelFrameClearPacket* packet = 
        command.getPacket<ChannelFrameClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );

    _setRenderContext( packet->context );
    frameClear( packet->context.frameID );
    _context = 0;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDraw( net::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW, this );

    _setRenderContext( packet->context );
    frameDraw( packet->context.frameID );
    _context = 0;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDrawFinish( net::Command& command )
{
    ChannelFrameDrawFinishPacket* packet = 
        command.getPacket< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( packet->frameID, packet->frameNumber );

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameAssemble( net::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getPacket<ChannelFrameAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );
    _setRenderContext( packet->context );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], _context->eye );
        _inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    for( FrameVector::const_iterator i = _inputFrames.begin();
         i != _inputFrames.end(); ++i )
    {
        // Unset the frame data on input frames, so that they only get flushed
        // once by the output frames during exit.
        (*i)->setData( 0 );
    }
    _inputFrames.clear();
    _context = 0;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameReadback( net::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getPacket<ChannelFrameReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_READBACK, this );
    _setRenderContext( packet->context );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], _context->eye );
        _outputFrames.push_back( frame );
    }

    frameReadback( packet->context.frameID );

    for( FrameVector::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i)
    {
        Frame* frame = *i;
        frame->setReady();
    }

    _outputFrames.clear();
    _context = 0;
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameTransmit( net::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << endl;

#ifndef EQ_ASYNC_TRANSMIT
    ChannelStatistics event( Statistic::CHANNEL_TRANSMIT, this );
#endif

    net::Session* session   = getSession();
    net::NodePtr  localNode = session->getLocalNode();
    net::NodePtr  server    = session->getServer();
    Pipe*         pipe      = getPipe();
    Frame*        frame     = pipe->getFrame( packet->frame,
                                              packet->context.eye );

    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        net::NodeID nodeID = packet->nodes[i];
        nodeID.convertToHost();

        net::NodePtr node   = command.getNode();
        net::NodePtr toNode = localNode->connect( nodeID, node );
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << endl;

#ifdef EQ_ASYNC_TRANSMIT
        getNode()->transmitter.send( frame->getData(), toNode, 
                                     getPipe()->getCurrentFrame( ));
#else
        ChannelStatistics nodeEvent( Statistic::CHANNEL_TRANSMIT_NODE, this );
        ChannelStatistics compressEvent( Statistic::CHANNEL_COMPRESS, this );
        compressEvent.event.statistic.endTime =
            compressEvent.event.statistic.startTime + frame->transmit( toNode );
#endif
    }

    return net::COMMAND_HANDLED;
}
}
