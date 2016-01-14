
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "channel.h"

#include "frameData.h"
#include "hlp.h"
#include "imageOrderer.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

namespace eVolve
{
Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _bgColor( eq::Vector3f::ZERO )
        , _taint( getenv( "EQ_TAINT_CHANNELS" ))
{
    _image.setAlphaUsage( true );
    _image.setInternalFormat( eq::Frame::BUFFER_COLOR,
                              EQ_COMPRESSOR_DATATYPE_RGBA );
}

static void checkError( const std::string& msg )
{
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        LBERROR << msg << " GL Error: " << error << std::endl;
}


bool Channel::configInit( const eq::uint128_t& initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    setNearFar( 0.001f, 10.0f );
    return true;
}

bool Channel::configExit()
{
    _image.resetPlugins();
    return eq::Channel::configExit();
}

void Channel::frameStart( const eq::uint128_t& frameID,
                          const uint32_t frameNumber )
{
    _image.reset();
    _bgColor = eq::Vector3f( 0.f, 0.f, 0.f );

    const BackgroundMode bgMode = _getFrameData().getBackgroundMode();

    if( bgMode == BG_WHITE )
        _bgColor = eq::Vector3f( 1.f, 1.f, 1.f );
    else
        if( bgMode == BG_COLOR || _taint )
             _bgColor = eq::Vector3f( getUniqueColor( )) / 255.f;

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameClear( const eq::uint128_t& )
{
    applyBuffer();
    applyViewport();

    if( getRange() == eq::Range::ALL )
        glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );
}


static void setLights( eq::Matrix4f& invRotationM )
{
    GLfloat lightAmbient[]  = {0.05f, 0.05f, 0.05f, 1.0f};
    GLfloat lightDiffuse[]  = {0.9f , 0.9f , 0.9f , 1.0f};
    GLfloat lightSpecular[] = {0.8f , 0.8f , 0.8f , 1.0f};
    GLfloat lightPosition[] = {1.0f , 1.0f , 1.0f , 0.0f};

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


static eq::Vector4f _getTaintColor( const ColorMode colorMode,
                                    const eq::Vector3f& color )
{
    if( colorMode == COLOR_MODEL )
        return eq::Vector4f::ZERO;

    eq::Vector4f taintColor( color.r(), color.g(), color.b(), 1.0 );
    const float alpha = ( colorMode == COLOR_HALF_DEMO ) ? 0.5 : 1.0;

    taintColor /= 255.f;
    taintColor      *= alpha;
    taintColor.a()   = alpha;
    return taintColor;
}

void Channel::frameDraw( const eq::uint128_t& )
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
    const eq::Matrix4f& rotation    = frameData.getRotation();
    const eq::Vector3f& translation = frameData.getTranslation();

    eq::Matrix4f     invRotationM;
    rotation.inverse( invRotationM );
    setLights( invRotationM );

    EQ_GL_CALL( applyHeadTransform( ));

    glTranslatef(  translation.x(), translation.y(), translation.z() );
    glMultMatrixf( rotation.array );

    Pipe*     pipe     = static_cast<Pipe*>( getPipe( ));
    Renderer* renderer = pipe->getRenderer();
    LBASSERT( renderer );

    const eq::Matrix4f& modelview = _computeModelView();

    // set fancy data colors
    const eq::Vector4f taintColor = _getTaintColor( frameData.getColorMode(),
                                                    getUniqueColor( ));
    const int normalsQuality = _getFrameData().getNormalsQuality();

    const eq::Range& range = getRange();
    renderer->render( range, modelview, invRotationM, taintColor,
                      normalsQuality );
    checkError( "error during rendering " );

    _image.setContext( getContext( ));

#ifndef NDEBUG
    outlineViewport();
#endif
}

bool Channel::useOrtho() const
{
    const FrameData& frameData = _getFrameData();
    return frameData.useOrtho();
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast< const Pipe* >( getPipe( ));
    return pipe->getFrameData();
}

eq::Matrix4f Channel::_computeModelView() const
{
    const FrameData& frameData = _getFrameData();
    const Pipe*      pipe      = static_cast< const Pipe* >( getPipe( ));
    const Renderer*  renderer  = pipe->getRenderer();
    eq::Matrix4f modelView( eq::Matrix4f::IDENTITY );

    if( renderer )
    {
        const VolumeScaling& volScaling = renderer->getVolumeScaling();

        eq::Matrix4f scale( eq::Matrix4f::ZERO );
        scale.at(0,0) = volScaling.W;
        scale.at(1,1) = volScaling.H;
        scale.at(2,2) = volScaling.D;
        scale.at(3,3) = 1.f;

        modelView = scale * frameData.getRotation();
    }
    modelView.set_translation( frameData.getTranslation( ));
    return getHeadTransform() * modelView;
}


static void _expandPVP( eq::PixelViewport& pvp, const eq::Images& images,
                        const eq::Vector2i& offset )
{
    for( eq::Images::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const eq::PixelViewport imagePVP = (*i)->getPixelViewport() + offset;
        pvp.merge( imagePVP );
    }
}


void Channel::clearViewport( const eq::PixelViewport &pvp )
{
    // clear given area
    glScissor( pvp.x, pvp.y, pvp.w, pvp.h );

    if( _image.getContext().range == eq::Range::ALL )
        glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );

    // restore assembly state
    const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();
    glScissor( 0, 0, windowPVP.w, windowPVP.h );
}

void Channel::_orderImages( eq::ImageOps& images )
{
    const FrameData&    frameData = _getFrameData();
    const eq::Matrix4f& rotation  = frameData.getRotation();
    const eq::Matrix4f& modelView = useOrtho() ? eq::Matrix4f::IDENTITY :
                                                 _computeModelView();

    // calculate modelview inversed+transposed matrix
    eq::Matrix3f modelviewITM;
    eq::Matrix4f modelviewIM;
    modelView.inverse( modelviewIM );
    eq::Matrix3f( modelviewIM ).transpose_to( modelviewITM );

    orderImages( images, modelView, modelviewITM, rotation, useOrtho( ));
}


void Channel::frameAssemble( const eq::uint128_t&, const eq::Frames& frames )
{
    _startAssemble();

    eq::PixelViewport  coveredPVP;
    eq::ImageOps       dbImages;
    eq::Zoom           zoom( eq::Zoom::NONE );

    // Make sure all frames are ready and gather some information on them
    for( eq::Frame* frame : frames )
    {
        {
            eq::ChannelStatistics stat( eq::Statistic::CHANNEL_FRAME_WAIT_READY,
                                        this );
            frame->waitReady( );
        }
        for( eq::Image* image : frame->getImages( ))
        {
            eq::ImageOp op( frame, image );
            op.offset = frame->getOffset();
            const eq::Range& range = image->getContext().range;
            if( range == eq::Range::ALL ) // 2D frame, assemble directly
                eq::Compositor::assembleImage( op, this );
            else
            {
                dbImages.emplace_back( op );
                zoom = frame->getZoom();
                _expandPVP( coveredPVP, frame->getImages(), frame->getOffset());
            }
        }
    }
    if( dbImages.empty( ))
    {
        resetAssemblyState();
        return;
    }

    // calculate correct image sequence
    const bool dbCompose = _image.getContext().range != eq::Range::ALL;
    coveredPVP.intersect( getPixelViewport( ));
    if( dbCompose && coveredPVP.hasArea( ))
    {
        eq::ImageOp op;
        op.image = &_image;
        op.buffers = eq::Frame::BUFFER_COLOR;
        op.zoom = zoom;
        op.offset = eq::Vector2i( coveredPVP.x, coveredPVP.y );
        dbImages.emplace_back( op );
    }

    _orderImages( dbImages );

    // Update range
    eq::Range newRange( 1.f, 0.f );
    for( const eq::ImageOp& op : dbImages )
    {
        const eq::Range& range = op.image->getContext().range;
        if( newRange.start > range.start ) newRange.start = range.start;
        if( newRange.end   < range.end   ) newRange.end   = range.end;
    }

    // check if current image is in proper position, read back if not
    if( dbCompose )
    {
        if( _bgColor == eq::Vector3f::ZERO && dbImages.front().image == &_image)
            dbImages.erase( dbImages.begin( ));
        else if( coveredPVP.hasArea())
        {
            eq::util::ObjectManager& glObjects = getObjectManager();
            eq::PixelViewport pvp = getRegion();
            pvp.intersect( coveredPVP );

            if( _image.startReadback( eq::Frame::BUFFER_COLOR, pvp,
                                      getContext(), zoom, glObjects ))
            {
                _image.finishReadback( glewGetContext( ));
            }
            clearViewport( coveredPVP );
        }
    }

    eq::Compositor::blendImages( dbImages, this, 0 );
    resetAssemblyState();
}

void Channel::_startAssemble()
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
}

void Channel::frameReadback( const eq::uint128_t& frameID,
                             const eq::Frames& frames )
{
    // Drop depth buffer flag from all output frames
    const FrameData& frameData = _getFrameData();
    for( eq::FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        frame->setQuality( eq::Frame::BUFFER_COLOR, frameData.getQuality());
        frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
    }

    eq::Channel::frameReadback( frameID, frames );
}

void Channel::frameViewFinish( const eq::uint128_t& frameID )
{
    _drawHelp();
    _drawLogo();
    eq::Channel::frameViewFinish( frameID );
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

    if( !frameData.showHelp() && message.empty( ))
        return;

    applyOverlayState();

    if( frameData.showHelp( ))
    {
        const eq::util::BitmapFont* font = getWindow()->getSmallFont();
        std::string help = EVolve::getHelp();
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

    if( !message.empty( ))
    {
        const eq::util::BitmapFont* font = getWindow()->getMediumFont();

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

    resetOverlayState();
}
}
