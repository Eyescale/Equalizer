
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

#ifndef EVOLVE_CHANNEL_H
#define EVOLVE_CHANNEL_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>

namespace eVolve
{
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );

protected:
    virtual ~Channel() {}

    bool configInit( const eq::uint128_t& initID ) override;
    bool configExit() override;

    void frameStart( const eq::uint128_t& frameID,
                     const uint32_t frameNumber ) override;

    void frameDraw( const eq::uint128_t& frameID ) override;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) override;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) override;
    void frameViewFinish( const eq::uint128_t& frameID ) override;
    void frameClear( const eq::uint128_t& frameID ) override;

    bool useOrtho() const override;

    void clearViewport( const eq::PixelViewport &pvp );

private:
    void _startAssemble();

    void _orderImages( eq::ImageOps& images );

    eq::Matrix4f _computeModelView() const;

    const FrameData& _getFrameData() const;

    void _drawLogo();
    void _drawHelp();

    eq::Vector3f _bgColor;   //!< background color
    eq::Image    _image;     //!< Readback buffer for DB compositing
    const bool   _taint;     //!< True if EQ_TAINT_CHANNELS is set
};

}

#endif // EVOLVE_CHANNEL_H
