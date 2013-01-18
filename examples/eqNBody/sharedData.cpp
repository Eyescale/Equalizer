
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "sharedData.h"
#include "sharedDataProxy.h"
#include "pipe.h"
#include "config.h"
#include "controller.h"

namespace eqNbody
{
SharedData::SharedData( Config *cfg ) : _cfg( cfg )
{
    LBASSERT( _cfg );
}

SharedData::~SharedData()
{
    if( _cfg )
        _cfg = 0;

    for( uint32_t i=0; i< _frameData.getNumDataProxies(); i++ )
    {
        delete _proxies[i];
    }
    _proxies.clear();
}

void SharedData::registerMemory( const eq::Range& range )
{
    // Initialise the local proxy
    unsigned int offset = range.start * _frameData.getNumBodies() * 4;
    unsigned int numBytes = ( range.end - range.start ) *
                            _frameData.getNumBytes();
    SharedDataProxy *shMem = new SharedDataProxy();
    _proxies.push_back( shMem );

    shMem->init( offset, numBytes, _frameData.getPos(), _frameData.getVel(),
                 _frameData.getCol() );

    // Register the proxy object
    _cfg->registerObject( shMem );

    const eq::uint128_t version = shMem->commit();

    // Let the app know which range is covered by this proxy
    _sendEvent( DATA_CHANGED, version, shMem->getID(), range );
}

void SharedData::mapMemory()
{
    const SharedDataProxy *shMem = _proxies[0];

    // Initialise the remote shared memory proxies
    for( uint32_t i=0; i< _frameData.getNumDataProxies(); i++)
    {
        const eq::uint128_t& pid = _frameData.getProxyID(i);

        if( pid != shMem->getID() )
        {
            SharedDataProxy *readMem = new SharedDataProxy();

            readMem->init( _frameData.getPos(), _frameData.getVel(),
                           _frameData.getCol() );
            _proxies.push_back( readMem );

            LBCHECK( _cfg->mapObject( readMem, pid ));
        }
    }
}

void SharedData::releaseMemory()
{
    for( uint32_t i=0; i< _frameData.getNumDataProxies(); i++ )
    {
        _cfg->releaseObject( _proxies[i] );
    }
}

void SharedData::syncMemory()
{
    for(unsigned int i=1; i< _frameData.getNumDataProxies(); i++)
    {
        const eq::uint128_t& pid = _proxies[i]->getID();
        const eq::uint128_t version = _frameData.getVersionForProxyID(pid);

        // ...and sync!
        _proxies[i]->sync( version );
    }
}

void SharedData::updateMemory(const eq::Range& range, Controller *controller)
{
    SharedDataProxy *local = _proxies[0];

    controller->getArray(BODYSYSTEM_POSITION, *local);
    controller->getArray(BODYSYSTEM_VELOCITY, *local);

    // Commit the local changes
    const eq::uint128_t version = local->commit();

    // Tell the others what version to sync.
    _sendEvent( PROXY_CHANGED, version, local->getID(), range) ;
}

void SharedData::_sendEvent( ConfigEventType type, const eq::uint128_t& version,
                             const eq::uint128_t& pid, const eq::Range& range )
{
    _cfg->sendEvent( type ) << pid << range << version;
}
}
