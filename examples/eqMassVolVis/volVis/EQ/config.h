
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2012, David Steiner   <steiner@ifi.uzh.ch>
 */

#ifndef MASS_VOL__CONFIG_H
#define MASS_VOL__CONFIG_H

#include "localInitData.h" // member
#include "frameData.h"     // member
#include "volumeInfo.h"

#include "cameraAnimation.h"

#include <eq/eq.h>

namespace massVolVis
{

class GUINode;

/**
 * Runtime user events handling.
 */
class Config : public eq::Config
{
public:
    Config( eq::ServerPtr parent );

    /** @sa eq::Config::init. */
    virtual bool init();
    /** @sa eq::Config::exit. */
    virtual bool exit();

    /** @sa eq::Config::startFrame. */
    virtual uint32_t startFrame();

    void setInitData( const LocalInitData& data ) { _initData = data; }
    const InitData& getInitData() const { return _initData; }

    /** Map per-config data to the local node process */
    bool mapData( const eq::uint128_t& initDataId );

    /** @return the current animation frame number. */
    uint32_t getAnimationFrame() { return _animation.getCurrentFrame(); }

protected:
    virtual ~Config();

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( eq::EventICommand command );

    int  _spinX, _spinY;
    int  _advance;
    bool _forceUpdate;

    eq::Canvas* _currentCanvas;

    LocalInitData _initData;
    FrameData     _frameData;
    VolumeInfo    _volumeInfo;

    uint64_t      _messageTime;

    friend class GUINode;

    VolumeInfo& getVolumeInfo() { return _volumeInfo; }

private:
    void _resetMessage();
    void _setMessage( const std::string& message );
    void _switchLayout( int32_t increment );
    void _deregisterData();
    bool _handleKeyEvent( const eq::KeyEvent& event );
    
    eq::View* _getCurrentView();
    const eq::View* _getCurrentView() const;

    CameraAnimation _animation;
};


}//namespace massVolVis

#endif // MASS_VOL__CONFIG_H
