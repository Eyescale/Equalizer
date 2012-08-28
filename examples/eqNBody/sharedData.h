
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


#ifndef EQNBODY_SHAREDDATA_H
#define EQNBODY_SHAREDDATA_H

#include <eq/eq.h>
#include <vector>

#include "frameData.h"
#include "configEvent.h"

namespace eqNbody
{
    class SharedDataProxy;
    class Config;
    class Controller;

    class SharedData
    {
    public:

        SharedData(Config *cfg);
        virtual ~SharedData();

        const FrameData& getFrameData() const { return _frameData; }

        void registerMemory( const eq::Range& range );
        void releaseMemory();
        void mapMemory();
        void syncMemory();
        void updateMemory( const eq::Range& range, Controller *controller );

        float* getPos() { return _frameData.getPos(); }
        float* getCol() { return _frameData.getCol(); }
        float* getVel() { return _frameData.getVel(); }

        float  getTimeStep() { return _frameData.getTimeStep(); }
        uint32_t getNumBytes() { return _frameData.getNumBytes(); }

        bool useStatistics() const { return _frameData.useStatistics(); }

    protected:

    private:
        void _sendEvent( ConfigEventType type, const eq::uint128_t& version,
                         const eq::uint128_t& pid, const eq::Range& range);

        std::vector< SharedDataProxy* >    _proxies;
        FrameData _frameData;
        Config*   _cfg;
    };
}

#endif // EQNBODY_SHAREDDATA_H
