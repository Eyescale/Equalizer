
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOLL__FRAMEDATA_H
#define MASS_VOLL__FRAMEDATA_H

#include "volVis.h"

#include <eq/eq.h>

namespace massVolVis
{


class FrameData : public co::Serializable
{
public:

    FrameData();

    void reset();

    /** @name Rendering flags. */
    //*{
    void adjustQuality( const float delta );
    void toggleHelp();
    void toggleRecording();
    void toggleStatistics();

    void toggleBackground();
    void toggleNormalsQuality();
    void toggleColorMode();
    void toggleBoundingBoxesDisplay();
    void togglePilotMode();

    void makeScreenshot();

    void increaseBudget();
    void decreaseBudget();

    void increaseError();
    void decreaseError();

    void spinCamera( const float x, const float y );
    void spinModel(  const float x, const float y, const float z );
    void moveCamera( const float x, const float y, const float z );

    void resetCameraSpin();

    void setCameraRotation( const eq::Vector3f& rotation    );
    void setCameraPosition( const eq::Vector3f& position );
    void setModelRotation(  const eq::Vector3f& rotation    );

    void setMaxTreeDepth( uint8_t value );

    bool showHelp()      const { return _help;       }
    bool useRecording()  const { return _recording;  }
    bool usePilotMode()  const { return _pilotMode; }
    bool useStatistics() const { return _statistics; }
    bool displayBoundingBoxes() const { return _boundingBoxes; }

    const eq::Matrix4f& getCameraRotation() const { return _cameraRotation; }
    const eq::Matrix4f& getModelRotation() const  { return _modelRotation; }
    const eq::Vector3f& getCameraPosition() const { return _cameraPosition; }

    float          getQuality()         const { return _quality;        }
    ColorMode      getColorMode()       const { return _colorMode;      }
    BackgroundMode getBackgroundMode()  const { return _bgMode;         }
    NormalsQuality getNormalsQuality()  const { return _normalsQuality; }
    uint32_t       getRenderingBudget() const { return _renderBudget;   }
    uint32_t       getScreenshotNumber()const { return _screenShot;     }
    uint8_t        getMaxTreeDepth()    const { return _maxTreeDepth;   }
    uint16_t       getRenderingError()  const { return _renderError;    }
    bool           useRenderingError()  const;

    float getCameraSpin()        const { return _cameraSpin;        }
    float getCameraTranslation() const { return _cameraTranslation; }
    //*}

    /** @name View interface. */
    //*{
    void setCurrentViewId( const eq::uint128_t& id );

    eq::uint128_t getCurrentViewId() const { return _currentViewId; }
    //*}

    /** @name Message overlay. */
    //*{
    void setMessage( const std::string& message );
    const std::string& getMessage() const { return _message; }
    //*}

protected:
    /** @sa Object::serialize() */
    virtual void serialize(   co::DataOStream& os, const uint64_t dirtyBits );

    /** @sa Object::deserialize() */
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );


    virtual ChangeType getChangeType() const { return DELTA; }

    /** The changed parts of the data since the last pack(). */
    enum DirtyBits
    {
        DIRTY_CAMERA  = co::Serializable::DIRTY_CUSTOM << 0,
        DIRTY_FLAGS   = co::Serializable::DIRTY_CUSTOM << 1,
        DIRTY_VIEW    = co::Serializable::DIRTY_CUSTOM << 2,
        DIRTY_MESSAGE = co::Serializable::DIRTY_CUSTOM << 3,
    };

private:

    eq::Matrix4f    _cameraRotation;
    eq::Matrix4f    _modelRotation;
    eq::Vector3f    _cameraPosition;

    uint8_t         _maxTreeDepth;
    float           _cameraSpin;
    float           _cameraTranslation;
    ColorMode       _colorMode;
    BackgroundMode  _bgMode;
    bool            _pilotMode;
    NormalsQuality  _normalsQuality;
    bool            _recording;
    bool            _statistics;
    bool            _boundingBoxes;
    bool            _help;
    float           _quality;
    eq::uint128_t   _currentViewId;
    std::string     _message;
    uint32_t        _renderBudget;
    uint32_t        _screenShot;
    uint16_t        _renderError;
};


}//namespace massVolVis


#endif // EVOLVE_FRAMEDATA_H

