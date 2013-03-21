
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

#ifndef EQNBODY_CONFIG_H
#define EQNBODY_CONFIG_H

#include <eq/eq.h>

#include "initData.h"
#include "frameData.h"
#include "configEvent.h"

namespace eqNbody
{
    class Config : public eq::Config
    {
    public:
        Config( eq::ServerPtr parent );

        virtual bool init();
        virtual bool exit();

        virtual uint32_t startFrame();

        void setInitData( const InitData& data ) { _initData = data; }
        const InitData& getInitData() const { return _initData; }

        void mapData( const eq::uint128_t& initDataID );
        void releaseData();

        virtual bool handleEvent( eq::EventICommand command );
        virtual bool handleEvent( const eq::ConfigEvent* event );
        bool needsRedraw();

    protected:
        virtual ~Config() {}

    private:
        InitData    _initData;
        FrameData   _frameData;
        bool        _redraw;

        bool _readyToCommit();
        bool _handleKeyEvent( const eq::KeyEvent& event );

        void _updateSimulation();
        void _registerData( eq::EventICommand& command );
        void _updateData( eq::EventICommand& command );
        void _deregisterData();
    };
}

#endif // EQ_PLY_CONFIG_H
