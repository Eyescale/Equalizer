//std12

#include "screenGrabber.h"

#include <msv/util/str.h>  // toString
#include <msv/util/pngwriter.h>
#include <msv/util/fileIO.h> // paddFilename

#include <lunchbox/debug.h>
#include <lunchbox/clock.h>
#include <eq/client/gl.h>
#include <eq/util/texture.h>
#include <eq/fabric/pixelViewport.h>

namespace massVolVis
{

ScreenGrabber::Frame::Frame()
    : w(0)
    , h(0)
    , b(0)
{
}

ScreenGrabber::Frame::Frame( int w_, int h_, int b_ )
{
    resize( w_, h_, b_ );
}

void ScreenGrabber::Frame::resize( int w_, int h_, int b_ )
{
    LBASSERT( w_ > 0 && h_ > 0 && b_ > 0 );
    w = w_;
    h = h_;
    b = b_;
    data.resize( w_*h_*b_ );
}


ScreenGrabber::ScreenGrabber( const GLEWContext* context )
    : _numBytes( 4 )
    , _frameNumber( 0 )
    , _glewContext( context )
    , _texturePtr( new eq::util::Texture( GL_TEXTURE_RECTANGLE_ARB, context ))
    , _clockPtr( new lunchbox::Clock( ))
{
    LBASSERT( context );
}


ScreenGrabber::~ScreenGrabber()
{
    _texturePtr->flush();
}


void ScreenGrabber::setNumBytes( int numBytes )
{
    LBASSERT( numBytes == 3 || numBytes == 4 );

    _numBytes = numBytes;
}


void ScreenGrabber::capture( const eq::fabric::PixelViewport& pvp )
{
    if( pvp.w <= 1 || pvp.h <= 1 )
        return;

    LBASSERT( _frameNumber  <= _frames.size( ));

    // add more memory if necessary
    if( _frameNumber == _frames.size( ))
        _frames.push_back( FrameSharedPtr( new Frame( pvp.w, pvp.h, _numBytes )));
    else
        _frames[ _frameNumber ]->resize( pvp.w, pvp.h, _numBytes );

    if( _frameNumber == 0 )
        _clockPtr->reset();

    _frames[ _frameNumber ]->time = _clockPtr->getTime64();

    uint8_t* dst = &(_frames[ _frameNumber ]->data[0]);

    _texturePtr->copyFromFrameBuffer(( _numBytes == 3 ? GL_RGB : GL_RGBA ), pvp );
    _texturePtr->download( dst );

    ++_frameNumber;
}


void ScreenGrabber::capture( int w, int h )
{
    capture( eq::fabric::PixelViewport( 0, 0, w, h ));
}


void ScreenGrabber::saveScreenshot( const GLEWContext* context, const std::string& nameTemplate, const eq::fabric::PixelViewport& pvp, int numBytes )
{
    LBASSERT( numBytes == 3 || numBytes == 4 );
    LBASSERT( context );

    static uint32_t _lastScreenshotNumber = 0;
    // get next free name
    std::string fName;
    do
    {
        fName = nameTemplate;
        util::paddFilename( fName, _lastScreenshotNumber );
        fName.append( ".png" );

        ++_lastScreenshotNumber;
    }while( util::fileSize( fName ) != 0 && _lastScreenshotNumber < 10000 );

    FrameSharedPtr frame( new Frame( pvp.w, pvp.h, numBytes ));
    uint8_t* dst = &(frame->data[0]);

    eq::util::Texture texture( GL_TEXTURE_RECTANGLE_ARB, context );
    texture.copyFromFrameBuffer(( numBytes == 3 ? GL_RGB : GL_RGBA ), pvp );
    texture.download( dst );
    texture.flush();

    _writeImage( frame, fName );
}


void ScreenGrabber::saveScreenshot( const GLEWContext* context, const std::string& nameTemplate, int w, int h, int numBytes )
{
    saveScreenshot( context, nameTemplate, eq::fabric::PixelViewport( 0, 0, w, h ), numBytes );
}


void ScreenGrabber::_writeImage( const FrameSharedPtr framePtr, const std::string& fileName )
{
    LBASSERT( framePtr );
    LBASSERT( static_cast<int32_t>(framePtr->data.size()) >= framePtr->w*framePtr->h*framePtr->b );

    LBWARN << "writing file: " << fileName.c_str() << std::endl;

    pngwriter png( framePtr->w, framePtr->h, 0, fileName.c_str() );
    for( int y = 0; y < framePtr->h; ++y )
        for( int x = 0; x < framePtr->w; ++x )
        {
            const uint8_t* rRef = &framePtr->data[ (y*framePtr->w + x)*framePtr->b ];
            double r = static_cast<double>(rRef[0]) / 255.0;
            double g = static_cast<double>(rRef[1]) / 255.0;
            double b = static_cast<double>(rRef[2]) / 255.0;
            png.plot( x, y, r, g, b );
        }
    png.close();
}


void ScreenGrabber::_writeImage( const FrameSharedPtr framePtr, const std::string& nameTemplate, uint32_t frameNum )
{
    std::string fName = nameTemplate;

    util::paddFilename( fName, frameNum );

    fName.append( ".png" );

    _writeImage( framePtr, fName );
}


void ScreenGrabber::saveFrames( const std::string& nameTemplate, int frameRate )
{
    LBWARN << "saving to: " << nameTemplate.c_str() << " framerate: " << frameRate << std::endl;

    if( frameRate <= 0 )
    {
        LBERROR << "frame rate has to be positive!" << std::endl;
        return;
    }

    if( _frameNumber == 0 )
    {
        LBWARN << "No frames were captured" << std::endl;
        return;
    }

    // during first  iteration check how many files would be written
    // during second iteration write files if no errors occure
    const uint32_t maxImageNum = 20*60*60; // 20 fps * 60 min
    for( int iter = 0; iter < 2; ++iter )
    {
        const float timeStep = 1000.f / static_cast<float>( frameRate ); // time step in ms
        float time = 0.f;

        uint32_t fPos     = 1;
        uint32_t imageNum = 0;
        while( fPos < _frameNumber && imageNum < maxImageNum )
        {
            int32_t nextTime = _frames[ fPos ]->time;
            while( nextTime > static_cast<int>( time ) && imageNum < maxImageNum )
            {
                // TODO: copy duplicated frames as file copies
                if( iter == 1 )
                    _writeImage( _frames[ fPos-1 ], nameTemplate, imageNum );
                time += timeStep;
                ++imageNum;
            }
            ++fPos;
        }
        if( imageNum >= maxImageNum )
        {
            LBWARN << "too many frames would be written, exiting without saving files" << std::endl;
            break;
        }
    }

    _frameNumber = 0;
}


void ScreenGrabber::preallocate( int w, int h, int nFrames )
{
    _frameNumber = 0;

    int oldSize = _frames.size();

    if( oldSize < nFrames )
    {
        _frames.resize( nFrames );
        for( int i = oldSize; i < nFrames; ++i )
            _frames[i] = FrameSharedPtr( new Frame( w, h, _numBytes ));
    }

    for( int i = 0; i < oldSize; ++i )
        _frames[i]->resize( w, h, _numBytes );
}


}

