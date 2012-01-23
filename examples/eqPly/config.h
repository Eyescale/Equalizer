
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

// members
#include "localInitData.h"
#include "frameData.h"
#include "tracker.h"
#include "cameraAnimation.h"

#include <eq/eq.h>
#include <eq/admin/base.h>

#include <set>

namespace eqPly
{
    class View;

    /**
     * The configuration, run be the EqPly application. 
     *
     * A configuration instance manages configuration-specific data: it
     * distributes the initialization and model data, updates frame-specific
     * data and manages frame generation based on event handling. 
     */
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

        /** Unmap per-config data to the local node process */
        void unmapData();

        /** @return the requested, default model or 0. */
        const Model* getModel( const eq::uint128_t& id );

        /** @sa eq::Config::handleEvent */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        /** @return true if the application is idling. */
        bool isIdleAA();

        /** @return true if an event required a redraw. */
        bool needRedraw();

        /** @return the current animation frame number. */
        uint32_t getAnimationFrame();

        /** @return the number of pipes having done a successful configInit. */
        size_t getNPipes() const { return _pipes.size(); }

    protected:
        virtual ~Config();

        /** Synchronize config and admin copy. */
        virtual co::uint128_t sync( 
                             const co::uint128_t& version = co::VERSION_HEAD );

    private:
        int         _spinX, _spinY;
        int         _advance;
        eq::Canvas* _currentCanvas;

        LocalInitData _initData;
        FrameData     _frameData;

        Tracker _tracker;

        Models     _models;
        ModelDists _modelDist;
        co::base::Lock  _modelLock;

        CameraAnimation _animation;

        uint64_t _messageTime;

        bool _redraw;
        bool _useIdleAA;

        int32_t _numFramesAA;

        eq::admin::ServerPtr _admin;

        std::set< eq::uint128_t > _pipes;

        void _loadModels();
        void _registerModels();
        void _loadPath();
        void _deregisterData();

        bool _needNewFrame();
        bool _handleKeyEvent( const eq::KeyEvent& event );

        void _switchCanvas();
        void _switchView();
        void _switchViewMode();
        void _switchModel();
        void _freezeLoadBalancing( const bool onOff );
        void _adjustTileSize( const int delta );
        void _switchLayout( int32_t increment );
        void _toggleEqualizer();

        void _setHeadMatrix( const eq::Matrix4f& matrix );
        const eq::Matrix4f& _getHeadMatrix() const;
        void _changeFocusDistance( const float delta );
        void _setFocusMode( const eq::FocusMode mode );

        /** @return a pointer to a connected admin server. */
        eq::admin::ServerPtr _getAdminServer();
        void _closeAdminServer();

        View* _getCurrentView();
        const View* _getCurrentView() const;

        void _setMessage( const std::string& message );
        void _updateData();
    };
}

#endif // EQ_PLY_CONFIG_H
