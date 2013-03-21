
#ifndef SCREEN_GRABBER_H
#define SCREEN_GRABBER_H

#include <msv/types/nonCopyable.h>
#include <msv/types/types.h>

#include <boost/shared_ptr.hpp>

#include <string>

namespace eq{ namespace util   { class Texture;       }}
namespace eq{ namespace fabric { class PixelViewport; }}
namespace lunchbox { class Clock; }

struct GLEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;


namespace massVolVis
{

class ScreenGrabber : private NonCopyable
{
public:
    ScreenGrabber( const GLEWContext* context );
    ~ScreenGrabber();

    void preallocate( int w, int h, int frames );

    /**
     * Set number of bytes for a pixel. Only 3 or 4 are supported
     * (8-bit RGB or RGBA), default is 4.
     */
    void setNumBytes( int numBytes );

    /**
     * Captures one frame from the screen to the memory buffer.
     * 
     * @param w width  of the viewport
     * @param h height of the viewport
     */
    void capture( int w, int h );

    void capture( const eq::fabric::PixelViewport& pvp );

    /**
     * Stores all captured frames as a sequence of video frames
     * of a specific frame rate (duplicates or removes images 
     * if necessary).
     * 
     * @param nameTemplate path and beginning of the file name
     * @param frameRate    how many frames per second is required
     */
    void saveFrames( const std::string& nameTemplate, int frameRate );

    /**
     * Makes and stores a screenshot as a unique file.
     */
    static void saveScreenshot( const GLEWContext* context, const std::string& nameTemplate, int w, int h, int numBytes = 4 );
    static void saveScreenshot( const GLEWContext* context, const std::string& nameTemplate, const eq::fabric::PixelViewport& pvp, int numBytes = 4 );

    uint32_t getFrameNumber() const { return _frameNumber; }

private:
     int32_t _numBytes;     //!< number of bytes per pixel (3 or 4)
    uint32_t _frameNumber;  //!< current frame number
    struct Frame
    {
        Frame();
        Frame( int w_, int h_, int b_ );
        void resize( int w_, int h_, int b_ );
        int w; // width
        int h; // height
        int b; // bytes per pixel
        int32_t time; // time stamp in ms
        std::vector<uint8_t> data;
    };

    typedef boost::shared_ptr< Frame > FrameSharedPtr;

    static void _writeImage( const FrameSharedPtr framePtr, const std::string& nameTemplate, uint32_t frameNum );
    static void _writeImage( const FrameSharedPtr framePtr, const std::string& fileName );

    const GLEWContext* _glewContext;
    const std::auto_ptr< eq::util::Texture > _texturePtr;
    const std::auto_ptr< lunchbox::Clock   > _clockPtr;

    std::vector< FrameSharedPtr > _frames;
};

}

#endif //SCREEN_GRABBER_H

