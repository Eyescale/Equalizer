
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EVOLVE_CONFIG_H
#define EVOLVE_CONFIG_H

#include <eq/eq.h>

#include "localInitData.h" // member
#include "frameData.h"     // member

namespace eVolve
{
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
        bool mapData( const eq::uint128_t& initDataID );

    protected:
        virtual ~Config();

        /** @sa eq::Config::handleEvent */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        int        _spinX, _spinY;

        eq::Canvas* _currentCanvas;

        LocalInitData _initData;
        FrameData     _frameData;

        uint64_t      _messageTime;

    private:
        void _resetMessage();
        void _setMessage( const std::string& message );
        void _switchLayout( int32_t increment );
        void _deregisterData();
        bool _handleKeyEvent( const eq::KeyEvent& event );

        static void _applyRotation( float m[16], const float dx,
                                                 const float dy );
    };
}

#endif // EVOLVE_CONFIG_H
