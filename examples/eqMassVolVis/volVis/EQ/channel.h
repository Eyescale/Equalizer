
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOL__CHANNEL_H
#define MASS_VOL__CHANNEL_H

#include <eq/eq.h>
#include <msv/types/vec2.h>
#include <msv/types/nonCopyable.h>


namespace massVolVis
{

class FrameData;
class ScreenGrabber;

/* */
class Channel : public eq::Channel, private NonCopyable
{
public:
    Channel( eq::Window* parent );
    virtual ~Channel();

    Vec2_f getScreenCenter();

    void drawFPS( const eq::PixelViewport& pvp, float fps );

protected:
    const FrameData& _getFrameData() const;

    virtual bool configInit( const eq::uint128_t& initId );
    virtual bool configExit();

    virtual void frameStart( const eq::uint128_t& frameID,
                             const uint32_t frameNumber );

    virtual void frameDraw(         const eq::uint128_t& frameId );
    virtual void frameAssemble(     const eq::uint128_t& frameId );
    virtual void frameReadback(     const eq::uint128_t& frameId );
    virtual void frameViewFinish(   const eq::uint128_t& frameId );
    virtual void frameClear(        const eq::uint128_t& frameId );

    void clearViewport( const eq::PixelViewport &pvp );

private:
    void _drawLogo();
    void _drawHelp();

    void _startAssemble();
    void _orderFrames( eq::Frames& frames );

    const eq::Vector3f _scaling; //!< Scaling of volume data in case voxels are not cubes

    eq::Vector3f _bgColor;  //!< background color
    eq::Frame    _frame;    //!< Readback buffer for DB compositing
    eq::Range    _drawRange;//!< The range from the last draw of this frame
    const bool   _taint;    //!< True if EQ_TAINT_CHANNELS is set

// frames capturing and recording data
    std::auto_ptr<ScreenGrabber> _screenGrabberPtr;

    bool _recording; //!< True if recording is in progress
    bool _recFDLast; //!< Last value of frameData's recording setting

    uint32_t _screenShotNum; //!< Number of screenshots made so far

    const static std::string _recordingPath;
    const static std::string _screenshotsPath;
    const static int         _recordingFrameRate;
};






} //namespace massVolVis

#endif //MASS_VOL__CHANNEL_H


