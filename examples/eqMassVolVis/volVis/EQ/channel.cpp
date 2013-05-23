
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "channel.h"

#include "../renderer/model.h"
#include "../LB/cameraParameters.h"
#include "../LB/boxN.h"

#include "volVis.h"

#include "pipe.h"
#include "node.h"
#include "window.h"
#include "frameData.h"
#include "screenGrabber.h"
#include <msv/util/str.h>

#include "../renderer/modelRenderParameters.h"

namespace massVolVis
{

const std::string Channel::_recordingPath   = std::string( "/home/steiner/imgs_"  );
const std::string Channel::_screenshotsPath = std::string( "/home/steiner/screen_");
const int         Channel::_recordingFrameRate = 20;

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _scaling( 1.0, 1.0, 1.0 )
        , _bgColor( eq::Vector3f::ZERO )
        , _drawRange( eq::Range::ALL )
        , _taint( getenv( "EQ_TAINT_CHANNELS" ))
        , _recording( false )
        , _recFDLast( false )
        , _screenShotNum( 0 )

{
}


Channel::~Channel()
{
    if( _screenGrabberPtr.get() )
        _screenGrabberPtr->saveFrames( _recordingPath, _recordingFrameRate );
}


bool Channel::configInit( const eq::uint128_t& initId )
{
    if( !eq::Channel::configInit( initId ))
        return false;

    setNearFar( 0.1f, 15.0f );

    eq::FrameDataPtr frameData = new eq::FrameData;
    frameData->setBuffers( eq::Frame::BUFFER_COLOR );
    _frame.setFrameData( frameData );


    return true;
}

bool Channel::configExit()
{
    // program crashes on exit, if GL Objects are not cleaned manually here
    _frame.getFrameData()->deleteGLObjects( getObjectManager() );
    _frame.getFrameData()->flush();

    return eq::Channel::configExit();
}


void Channel::frameStart( const eq::uint128_t& frameId,
                          const uint32_t frameNumber )
{
    _drawRange = eq::Range::ALL;
    _bgColor = eq::Vector3f( 0.f, 0.f, 0.f );

    const BackgroundMode bgMode = _getFrameData().getBackgroundMode();

    if( bgMode == BG_WHITE )
        _bgColor = eq::Vector3f( 1.f, 1.f, 1.f );
    else
        if( bgMode == BG_COLOR || _taint )
             _bgColor = eq::Vector3f( getUniqueColor( )) / 255.f;

    eq::Channel::frameStart( frameId, frameNumber );
}

void Channel::frameClear( const eq::uint128_t& )
{
    applyBuffer();
    applyViewport();

    _drawRange = getRange();
//    if( _drawRange == eq::Range::ALL )
//        glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
//    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast< const Pipe* >( getPipe( ));
    return pipe->getFrameData();
}


namespace
{
void _setLights( eq::Matrix4f& invRotationM )
{
    GLfloat lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 0.0f};

    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );

    // rotate light in the opposite direction of the model rotation to keep
    // light position constant and avoid recalculating normals in the fragment
    // shader
    glPushMatrix();
    glMultMatrixf( invRotationM.array );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
    glPopMatrix();
}


void _drawAxiss( const float size )
{
    const float ls  = size / 40.f;
    const float ls2 = ls*2.f;
    glDisable( GL_LIGHTING    );

    glLineWidth( 1.5 );
    glBegin( GL_LINES );
        // X
        glColor3f(   1.f,  0.f,  0.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f( size,  0.f,  0.f );

        glVertex3f( size+ls-ls, -ls,  0.f );
        glVertex3f( size+ls+ls,  ls,  0.f );
        glVertex3f( size+ls-ls,  ls,  0.f );
        glVertex3f( size+ls+ls, -ls,  0.f );

        // Y
        glColor3f(   0.f,  1.f,  0.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f(  0.f, size,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(   ls, size+ls2+ls,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(  -ls, size+ls2+ls,  0.f );

        glVertex3f(  0.f, size+ls2,  0.f );
        glVertex3f(  0.f, size+ls2-ls,  0.f );

        // Z
        glColor3f(   0.f,  0.f,  1.f );
        glVertex3f(  0.f,  0.f,  0.f );
        glVertex3f(  0.f,  0.f, size );
    glEnd();
    glBegin( GL_LINE_STRIP );
        glVertex3f( -ls,  ls, size+ls );
        glVertex3f(  ls,  ls, size+ls );
        glVertex3f( -ls, -ls, size+ls );
        glVertex3f(  ls, -ls, size+ls );
    glEnd();

    glEnable( GL_LIGHTING    );
}

float _getTaintAlpha( const ColorMode colorMode )
{
    switch( colorMode )
    {
        case COLOR_DEMO     : return 255.f;
        case COLOR_HALF_DEMO: return 128.f;
        case COLOR_NORMALS  : return   1.f;
        default             : return   0.f;
    }
}

eq::Vector4f _getTaintColor( const ColorMode colorMode,
                             const eq::Vector3ub& color )
{
    return eq::Vector4f( color.r(), color.g(), color.b(),
                                _getTaintAlpha( colorMode )) / 255.f;
}
}// namespace

void Channel::frameDraw( const eq::uint128_t& frameId )
{
    // Setup frustum
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));

    // Setup lights before applying head transform, so the light will be
    // consistent in the cave
    const FrameData&    frameData   = _getFrameData();

    const eq::Matrix4f& cameraRotation  = frameData.getCameraRotation();
    const eq::Matrix4f& modelRotation   = frameData.getModelRotation();
    const eq::Vector3f& cameraPosition  = frameData.getCameraPosition();

    eq::Matrix4f rotation = cameraRotation * modelRotation;
    eq::Matrix4f invRotationM;
    rotation.inverse( invRotationM );
    _setLights( invRotationM );

    EQ_GL_CALL( applyHeadTransform( ));

    glMultMatrixf( cameraRotation.array );
    glTranslatef( cameraPosition.x(), cameraPosition.y(), cameraPosition.z() );
    glMultMatrixf( modelRotation.array );

    EQ_GL_CALL( glScalef( _scaling.x(), _scaling.y(), _scaling.z() ));

    Pipe*  pipe = static_cast< Pipe* >( getPipe( ));

    // get Modelview matrix
    eq::Matrix4f modelviewM;
    eq::Matrix4f scale = eq::Matrix4f::IDENTITY; scale.scale( _scaling );
    modelviewM = scale * modelRotation;
    modelviewM.set_translation( cameraPosition );
    modelviewM = cameraRotation * modelviewM;
    modelviewM = getHeadTransform() * modelviewM;

    const eq::Vector3f viewVector =
                                eq::Vector3f( invRotationM.array[8],
                                              invRotationM.array[9],
                                              invRotationM.array[10] );

    // Compute cull matrix
    const eq::Frustumf& frustum     = getFrustum();
    const eq::Matrix4f projection   = frustum.compute_matrix();
    const eq::Matrix4f projectionMV = projection * modelviewM;

    const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();
    const Vec2_f screenSize( windowPVP.w, windowPVP.h );

//    const eq::Vector4d clip( 0, 0, -1, -0.5 );
//    EQ_GL_CALL( glClipPlane( GL_CLIP_PLANE0, clip.array ));
//    EQ_GL_CALL( glEnable( GL_CLIP_PLANE0 ));

//    EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));

    const eq::Range& range = getRange();
    const eq::Vector4f taintColor = _getTaintColor( frameData.getColorMode(),
                                                    getUniqueColor( ));

    ModelSPtr model = pipe->getModel();
    if( model )
    {
        clearViewport( getPixelViewport() );

        ModelRenderParameters rParams = { modelviewM, projectionMV, getScreenCenter(), screenSize,
                                                viewVector, range, taintColor,
                                                frameData.displayBoundingBoxes(),
                                                frameData.getRenderingBudget(),
                                                frameData.getMaxTreeDepth(),
                                                frameData.getCameraSpin(),
                                                frameData.getCameraTranslation(),
                                                frameData.useRenderingError(),
                                                frameData.getRenderingError(),
                                                this };
        model->render( rParams );

        const Rect_i32 r = model->getRegion();
        if( r.valid( ))
        {
            declareRegion( eq::PixelViewport( r.s.x, r.s.y, r.getWidth(), r.getHeight() ));
        }

        // TODO: Check that frame is not read to RAM
        // TODO: Check why explicit deleting of images is required
        // if front to back rendering -> replace BG
        if( false )//r.valid( ))
        {
            _startAssemble();
            eq::FrameDataPtr data = _frame.getFrameData();
            data->setRange( _drawRange );
            _frame.clear();

            eq::Frames dbFrames;
            dbFrames.push_back( &_frame );

            eq::Window::ObjectManager* glObjects = getObjectManager();

            _frame.setOffset( eq::Vector2i( 0, 0 ));
            _frame.setZoom( getZoom() );
            data->setPixelViewport( getPixelViewport() );

            _frame.readback( glObjects, getDrawableConfig(), getRegions( ));
            clearViewport( getPixelViewport() );
            // offset for assembly
            _frame.setOffset( eq::Vector2i( getPixelViewport().x, getPixelViewport().y ));

            // blend DB frames in order
            try
            {
                eq::Compositor::assembleFramesSorted( dbFrames, this, 0, true );
            }
            catch( const co::Exception& e )
            {
                LBWARN << e.what() << std::endl;
            }

            resetAssemblyState();
        }

        // capture screen if necessary
        if( frameData.getScreenshotNumber() > _screenShotNum )
        {
            _screenShotNum = frameData.getScreenshotNumber();
            ScreenGrabber::saveScreenshot( glewGetContext(), _screenshotsPath, screenSize.w, screenSize.h );
        }

        if( frameData.useRecording())
        {
            if( !_screenGrabberPtr.get() )
            {
                _screenGrabberPtr = std::auto_ptr<ScreenGrabber>( new ScreenGrabber( glewGetContext() ));
                _screenGrabberPtr->preallocate( screenSize.w, screenSize.h, 1000 );
            }
            if( _screenGrabberPtr->getFrameNumber() == 0 )
                _recording = true;

            if( _screenGrabberPtr->getFrameNumber() < 1200 )
                _screenGrabberPtr->capture( screenSize.w, screenSize.h );
            else
                _recording = false;
        }else
            if( _recFDLast && _screenGrabberPtr.get() )
            {
                _screenGrabberPtr->saveFrames( _recordingPath, _recordingFrameRate );
                _recording = false;
            }
    }
    else
        lunchbox::sleep( 100 );
//    _drawAxiss( .45 );

    _drawRange = range;
    _recFDLast = frameData.useRecording();
}


void Channel::frameViewFinish( const eq::uint128_t& frameId )
{
    _drawHelp();
//    _drawLogo();
}

namespace
{
void _expandPVP( eq::PixelViewport& pvp, const eq::Images& images,
                        const eq::Vector2i& offset )
{
    for( eq::Images::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const eq::PixelViewport imagePVP = (*i)->getPixelViewport() + offset;
        pvp.merge( imagePVP );
    }
}
}


void Channel::clearViewport( const eq::PixelViewport &pvp )
{
    // clear given area
    glScissor(  pvp.x, pvp.y, pvp.w, pvp.h );

    if( _drawRange == eq::Range::ALL )
        glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );

    // restore assembly state
    const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();
    glScissor( 0, 0, windowPVP.w, windowPVP.h );
}


Vec2_f Channel::getScreenCenter()
{
    const eq::PixelViewport& pvp = getPixelViewport();
    return Vec2_f( pvp.x+pvp.w/2, pvp.y+pvp.h/2 );
}


void Channel::_orderFrames( eq::Frames& frames )
{
    if( frames.size() < 1 )
    {
        LBWARN << "No frames to compose" << std::endl;
        return;
    }

    Pipe*  pipe = static_cast< Pipe* >( getPipe( ));

    ModelSPtr model = pipe->getModel();
    if( !model )
        return;

    constVolumeTreeBaseSPtr tree = model->getVolumeTree();
    if( !tree )
    {
        LBWARN << "pipe has tree" << std::endl;
        return;
    }

    // basically duplicate frameDraw code to get parameters
    // could be cached though ( modelviewM, projectionMV, screenSize )
    const FrameData&    frameData   = _getFrameData();
    const eq::Matrix4f& cameraRotation  = frameData.getCameraRotation();
    const eq::Matrix4f& modelRotation   = frameData.getModelRotation();
    const eq::Vector3f& cameraPosition  = frameData.getCameraPosition();

    eq::Matrix4f modelviewM;
    eq::Matrix4f scale = eq::Matrix4f::IDENTITY; scale.scale( _scaling );
    modelviewM = scale * modelRotation;
    modelviewM.set_translation( cameraPosition );
    modelviewM = cameraRotation * modelviewM;
    modelviewM = getHeadTransform() * modelviewM;

    const eq::Frustumf& frustum     = getFrustum();
    const eq::Matrix4f projection   = frustum.compute_matrix();
    const eq::Matrix4f projectionMV = projection * modelviewM;

    const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();  // TODO: check why not Channel::getPixelViewport()?
    const Vec2_f screenSize( windowPVP.w, windowPVP.h );

    CameraParameters cp( modelviewM, projectionMV, getScreenCenter(), screenSize );

    const float rangeSize = frames[0]->getRange().getSize();
    uint32_t dbSegmentsN = 1;
    if( rangeSize != eq::Range::ALL.getSize() )
        dbSegmentsN = (eq::Range::ALL.getSize() +rangeSize/2.0) / rangeSize;

    const vecBoxN_f& bbs = model->getDataSplit( dbSegmentsN, *tree, cp );

    eq::Frames tmpFrames = frames;
    frames.clear();
    // for each data part's id find the best fitting range among frames
    // if it exists
    for( size_t i = bbs.size(); i > 0; --i )
    {
        const float rangeStart = bbs[i-1].pos * rangeSize;
        for( size_t j = 0; j < tmpFrames.size(); ++j )
            if( fabs( rangeStart - tmpFrames[j]->getRange().start ) <
                                                                 rangeSize/4.f )
            {
                frames.push_back( tmpFrames[j] );
                break;
            }
        if( frames.size() >= tmpFrames.size() )
            break;
    }
    if( frames.size() < tmpFrames.size() )
        LBWARN << "number of output frames to composit is less than requested!"
               << std::endl;
}


void Channel::frameAssemble( const eq::uint128_t& )
{
    const bool composeOnly = (_drawRange == eq::Range::ALL);

    _startAssemble();

    const eq::Frames& frames = getInputFrames();
    eq::PixelViewport  coveredPVP;
    eq::Frames         dbFrames;
    eq::Zoom           zoom( eq::Zoom::NONE );

    // Make sure all frames are ready and gather some information on them
    for( eq::Frames::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        {
            eq::ChannelStatistics stat( eq::Statistic::CHANNEL_FRAME_WAIT_READY,
                                        this );
            frame->waitReady( );
        }
        const eq::Range& range = frame->getRange();
        if( range == eq::Range::ALL ) // 2D frame, assemble directly
            eq::Compositor::assembleFrame( frame, this );
        else
        {
            dbFrames.push_back( frame );
            zoom = frame->getZoom();
            _expandPVP( coveredPVP, frame->getImages(), frame->getOffset() );
        }
    }
    coveredPVP.intersect( getPixelViewport( ));

    if( dbFrames.empty() )
    {
        resetAssemblyState();
        return;
    }

    // calculate correct frames sequence
    eq::FrameDataPtr data = _frame.getFrameData();
    if( !composeOnly && coveredPVP.hasArea( ))
    {
        _frame.clear();
        data->setRange( _drawRange );
        dbFrames.push_back( &_frame );
    }

    _orderFrames( dbFrames );

    // Update range
    eq::Range newRange( 1.f, 0.f );
    for( size_t i = 0; i < dbFrames.size(); ++i )
    {
        const eq::Range range = dbFrames[i]->getRange();
        if( newRange.start > range.start ) newRange.start = range.start;
        if( newRange.end   < range.end   ) newRange.end   = range.end;
    }
    _drawRange = newRange;

    // check if current frame is in proper position, read back if not
    if( !composeOnly )
    {
        if( _bgColor == eq::Vector3f::ZERO && dbFrames.front() == &_frame &&
            !static_cast< Pipe* >( getPipe( ))->getModel()->isFrontToBackRendering())
        {
            std::cout << "not erasing main frame" << std::endl;
            dbFrames.erase( dbFrames.begin( ));
        }
        else if( coveredPVP.hasArea())
        {
            eq::Window::ObjectManager* glObjects = getObjectManager();

            _frame.setOffset( eq::Vector2i( 0, 0 ));
            _frame.setZoom( zoom );
            data->setPixelViewport( coveredPVP );
            _frame.readback( glObjects, getDrawableConfig(), getRegions( ));
            clearViewport( coveredPVP );

            // offset for assembly
            _frame.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ));
        }
    }

    // blend DB frames in order
    try
    {
        eq::Compositor::assembleFramesSorted( dbFrames, this, 0, 
                                              true /*blendAlpha*/ );
    }
    catch( const co::Exception& e )
    {
        LBWARN << e.what() << std::endl;
    }

    resetAssemblyState();
}


void Channel::_startAssemble()
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
}


void Channel::frameReadback( const eq::uint128_t& frameId )
{
    // Drop depth buffer flag from all output frames
    const eq::Frames& frames = getOutputFrames();
    const FrameData& frameData = _getFrameData();
    for( eq::FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        frame->setQuality( eq::Frame::BUFFER_COLOR, frameData.getQuality());
        frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
        frame->getFrameData()->setRange( _drawRange );
    }

    eq::Channel::frameReadback( frameId );
}


void Channel::_drawLogo( )
{
    // Draw the overlay logo
    const Window* window = static_cast<Window*>( getWindow( ));
    const eq::util::Texture* texture = window->getLogoTexture();
    if( !texture )
        return;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glColor3f( 1.0f, 1.0f, 1.0f );
    const GLenum target = texture->getTarget();
    glEnable( target );
    texture->bind();
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    const float tWidth = float( texture->getWidth( ) );
    const float tHeight = float( texture->getHeight( ) );

    const float width = target == GL_TEXTURE_2D ? 1.0f : tWidth;
    const float height = target == GL_TEXTURE_2D ? 1.0f : tHeight;

    glBegin( GL_QUADS ); {
        glTexCoord2f( 0, 0 );
        glVertex3f( 5.0f, 5.0f, 0.0f );

        glTexCoord2f( width, 0 );
        glVertex3f( tWidth + 5.0f, 5.0f, 0.0f );

        glTexCoord2f( width, height );
        glVertex3f( tWidth + 5.0f, tHeight + 5.0f, 0.0f );

        glTexCoord2f( 0, height );
        glVertex3f( 5.0f, tHeight + 5.0f, 0.0f );

    } glEnd();

    glDisable( target );
    glDisable( GL_BLEND );
}


void Channel::_drawHelp()
{
    const FrameData& frameData = _getFrameData();
    std::string message = frameData.getMessage();

    if( message.empty() && ( _screenGrabberPtr.get() && _screenGrabberPtr->getFrameNumber() > 0 ))
    {
        message = "Recording (" + strUtil::toString(_screenGrabberPtr->getFrameNumber()) + ")";
        if( !_recording )
            message.append( " - paused" );
        message.append( " \n Hit 'r' again to write files." );
    }

    if( !frameData.showHelp() && message.empty() )
        return;

    applyBuffer();
    applyViewport();
    setupAssemblyState();

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    if( _bgColor != eq::Vector3f( 1.f, 1.f, 1.f ))
        glColor3f( 1.f, 1.f, 1.f );
    else
        glColor3f( 0.f, 0.f, 0.f );

    if( frameData.showHelp( ))
    {
        const eq::Window::Font* font = getWindow()->getSmallFont();
        std::string help = VolVis::getHelp();
        float y = 340.f;

        for( size_t pos = help.find( '\n' ); pos != std::string::npos;
             pos = help.find( '\n' ))
        {
            glRasterPos3f( 10.f, y, 0.99f );

            font->draw( help.substr( 0, pos ));
            help = help.substr( pos + 1 );
            y -= 16.f;
        }
        // last line
        glRasterPos3f( 10.f, y, 0.99f );
        font->draw( help );
    }

    if( !message.empty() )
    {
        const eq::Window::Font* font = getWindow()->getMediumFont();

        const eq::Viewport& vp = getViewport();
        const eq::PixelViewport& pvp = getPixelViewport();

        const float width = pvp.w / vp.w;
        const float xOffset = vp.x * width;

        const float height = pvp.h / vp.h;
        const float yOffset = vp.y * height;
        const float yMiddle = 0.5f * height;
        float y = yMiddle - yOffset;

        for( size_t pos = message.find( '\n' ); pos != std::string::npos;
             pos = message.find( '\n' ))
        {
            glRasterPos3f( 10.f - xOffset, y, 0.99f );

            font->draw( message.substr( 0, pos ));
            message = message.substr( pos + 1 );
            y -= 22.f;
        }
        // last line
        glRasterPos3f( 10.f - xOffset, y, 0.99f );
        font->draw( message );
    }

    EQ_GL_CALL( resetAssemblyState( ));
}


void Channel::drawFPS( const eq::PixelViewport& pvp, float fps )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    std::ostringstream fpsText;
    fpsText << std::setprecision(3) << fps << " FPS";

    const eq::Window::Font* font = getWindow()->getSmallFont();

    if( _bgColor == eq::Vector3f( 1.f, 1.f, 1.f ))
    {
        glColor3f( 0.f, 0.f, 0.f );
        glRasterPos3f( pvp.w - 60.f, 10.f , 0.99f );
        font->draw( fpsText.str( ));
    }else
        glColor3f( 1.f, 1.f, 1.f );

    ModelSPtr model = static_cast< Pipe* >( getPipe( ))->getModel();
    if( model )
    {
        glRasterPos3f( pvp.w - 60.f, 30.f , 0.99f );
        font->draw( strUtil::toString( model->bricksRendered() ));
    }

    EQ_GL_CALL( resetAssemblyState( ));
}


} // namespace massVolVis

