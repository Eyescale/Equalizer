
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

namespace eqPly
{
/**
 * Frame-specific data.
 *
 * The frame-specific data is used as a per-config distributed object and
 * contains mutable, rendering-relevant data. Each rendering thread (pipe) keeps
 * its own instance synchronized with the frame currently being rendered. The
 * data is managed by the Config, which modifies it directly.
 */
class FrameData : public co::Serializable
{
public:
    FrameData();

    virtual ~FrameData() {}

    void reset();

    /** @name Rendering flags. */
    //*{
    void setModelID( const eq::uint128_t& id );

    void setColorMode( const ColorMode color );
    void setRenderMode( const triply::RenderMode mode );
    void setIdle( const bool idleMode );

    void toggleOrtho();
    void toggleStatistics();
    void toggleHelp();
    void toggleWireframe();
    void toggleColorMode();
    void adjustQuality( const float delta );
    void togglePilotMode();
    triply::RenderMode toggleRenderMode();
    void toggleCompression();

    eq::uint128_t getModelID() const { return _modelID; }
    ColorMode getColorMode() const { return _colorMode; }
    float getQuality() const { return _quality; }
    bool useOrtho() const { return _ortho; }
    bool useStatistics() const { return _statistics; }
    bool showHelp() const { return _help; }
    bool useWireframe() const { return _wireframe; }
    bool usePilotMode() const { return _pilotMode; }
    bool isIdle() const { return _idle; }
    triply::RenderMode getRenderMode() const { return _renderMode; }
    bool useCompression() const { return _compression; }
    //*}

    /** @name Camera parameters. */
    //*{
    void setCameraPosition( const eq::Vector3f& position );
    void setRotation( const eq::Vector3f& rotation);
    void setModelRotation( const eq::Vector3f& rotation    );
    void spinCamera( const float x, const float y );
    void spinModel(  const float x, const float y, const float z );
    void moveCamera( const float x, const float y, const float z );

    const eq::Matrix4f& getCameraRotation() const
        { return _rotation; }
    const eq::Matrix4f& getModelRotation() const
        { return _modelRotation; }
    const eq::Vector3f& getCameraPosition() const
        { return _position; }
    //*}

    /** @name View interface. */
    //*{
    void setCurrentViewID( const eq::uint128_t& id );
    eq::uint128_t getCurrentViewID() const { return _currentViewID; }
    //*}

    /** @name Message overlay. */
    //*{
    void setMessage( const std::string& message );
    const std::string& getMessage() const { return _message; }
    //*}

protected:
    /** @sa Object::serialize() */
    virtual void serialize( co::DataOStream& os,
                            const uint64_t dirtyBits );
    /** @sa Object::deserialize() */
    virtual void deserialize( co::DataIStream& is,
                              const uint64_t dirtyBits );

    virtual ChangeType getChangeType() const { return DELTA; }

    /** The changed parts of the data since the last pack(). */
    enum DirtyBits
    {
        DIRTY_CAMERA  = co::Serializable::DIRTY_CUSTOM << 0,
        DIRTY_FLAGS   = co::Serializable::DIRTY_CUSTOM << 1,
        DIRTY_VIEW    = co::Serializable::DIRTY_CUSTOM << 2,
        DIRTY_MESSAGE = co::Serializable::DIRTY_CUSTOM << 3
    };

private:
    eq::Matrix4f _rotation;
    eq::Matrix4f _modelRotation;
    eq::Vector3f _position;

    eq::uint128_t    _modelID;
    triply::RenderMode  _renderMode;
    ColorMode        _colorMode;
    float            _quality;
    bool             _ortho;
    bool             _statistics;
    bool             _help;
    bool             _wireframe;
    bool             _pilotMode;
    bool             _idle;
    bool             _compression;

    eq::uint128_t _currentViewID;
    std::string _message;
};
}


#endif // EQ_PLY_FRAMEDATA_H
