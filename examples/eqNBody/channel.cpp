/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
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

#include "client.h"
#include "initData.h"
#include "config.h"
#include "pipe.h"

#include "controller.h"
#include "sharedData.h"

using namespace lunchbox;
using namespace std;

namespace eqNbody
{
Channel::Channel( eq::Window* parent )
    : eq::Channel( parent )
    , _controller( new Controller( glewGetContext( )))
    , _registerMem( true )
    , _mapMem( true )
{}

Channel::~Channel()
{
    delete _controller;
}

bool Channel::configInit( const eq::uint128_t& initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    // Initialize the CUDA controller
    const InitData& id = static_cast<Config*>( getConfig() )->getInitData();
    SharedData& sd = static_cast<Pipe*>( getPipe() )->getSharedData();

    LBCHECK( _controller->init(id, sd.getPos(), true ));
    return true;
}

void Channel::frameDraw( const eq::uint128_t& frameID )
{
    const eq::Range& range = getRange();
    SharedData& sd = static_cast<Pipe*>( getPipe() )->getSharedData();

    // 1st, register the local memory
    if( _registerMem )
    {
        sd.registerMemory( getRange() );
        _registerMem = false;

        // Make sure all proxies are mapped before cont'ing
        return;
    }

    // 2nd, map remote memory
    if( _mapMem )
    {
        sd.mapMemory();
        _mapMem = false;
    }

    // 3rd, synchronize the shared memory
    sd.syncMemory();

    // 4th, update the GPU memory and run one simulation step
    const uint32_t nBytes = sd.getNumBytes();
    _controller->setArray( BODYSYSTEM_POSITION, sd.getPos(), nBytes );
    _controller->setArray( BODYSYSTEM_VELOCITY, sd.getVel(), nBytes );
    _controller->compute( sd.getTimeStep(), range );

    // 5th, draw the stars
    eq::Channel::frameDraw( frameID );
    _controller->draw( sd.getPos(), sd.getCol() );

#ifndef NDEBUG
    outlineViewport();
#endif

    // Finally, redistribute the newly computed data from the GPU to all
    // interested mappers
    sd.updateMemory( range, _controller );
}
}
