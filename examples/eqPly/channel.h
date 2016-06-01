
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include <eq/eq.h>


namespace eqPly
{
/** The rendering entity, updating a part of a Window. */
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );

    bool stopRendering() const;

protected:
    virtual ~Channel() {}

    bool configInit( const eq::uint128_t& initID ) override;
    void frameClear( const eq::uint128_t& frameID ) override;
    void frameDraw( const eq::uint128_t& frameID ) override;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) override;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) override;
    void frameStart( const eq::uint128_t&, const uint32_t ) override;
    void frameFinish( const eq::uint128_t&, const uint32_t ) override;
    void frameViewStart( const eq::uint128_t& frameID ) override;
    void frameViewFinish( const eq::uint128_t& frameID ) override;

    bool useOrtho() const override;
    eq::Vector2f getJitter() const override;

    void notifyStopFrame( const uint32_t lastFrameNumber ) override
    { _frameRestart = lastFrameNumber + 1; }

private:
    void _drawModel( const Model* model );
    void _drawOverlay();
    void _drawHelp();
    void _updateNearFar( const triply::BoundingSphere& boundingSphere );

    bool _isDone() const;

    void _initJitter();
    bool _initAccum();

    /** the subpixel for this step. */
    eq::Vector2i _getJitterStep() const;

    const FrameData& _getFrameData() const;
    const Model*     _getModel();

    const Model* _model;
    eq::uint128_t _modelID;
    uint32_t _frameRestart;

    struct Accum
    {
        Accum() : step( 0 ), stepsDone( 0 ), transfer( false )
        {}

        std::unique_ptr< eq::util::Accum > buffer;
        int32_t step;
        uint32_t stepsDone;
        bool transfer;
    }
        _accum[ eq::NUM_EYES ];

    eq::PixelViewport _currentPVP;
};
}

#endif // EQ_PLY_CHANNEL_H
