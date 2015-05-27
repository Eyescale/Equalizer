
/* Copyright (c) 2009-2015, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_CPU_CHANNEL_H
#define EQ_CPU_CHANNEL_H

#include <eq/channel.h> // base class

namespace eqCpu
{

class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );
    virtual ~Channel() {}

protected:
    bool configInit( const eq::uint128_t& initID ) final
        { return eq::Channel::configInit( initID ); }
    bool configExit() final { return eq::Channel::configExit(); }

    void frameDraw( const eq::uint128_t& frameID ) final;
    void frameClear( const eq::uint128_t& frameID ) final;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) final;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) final;

    void setupAssemblyState() final;
    void resetAssemblyState() final;
};
}

#endif // EQ_CPU_CHANNEL_H
