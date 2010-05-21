
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

#include <eq/eq.h>
#include <eq/admin.h>

// members
#include "localInitData.h"
#include "frameData.h"
#include "tracker.h"
#include "cameraAnimation.h"

namespace eqPly
{
    typedef std::vector< Model* >     Models;
    typedef std::vector< ModelDist* > ModelDists;

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
        Config( eq::base::RefPtr< eq::Server > parent );

        /** @sa eq::Config::init. */
        virtual bool init();
        /** @sa eq::Config::exit. */
        virtual bool exit();

        /** @sa eq::Config::startFrame. */
        virtual uint32_t startFrame();

        void setInitData( const LocalInitData& data ) { _initData = data; }
        const InitData& getInitData() const { return _initData; }

        /** Map per-config data to the local node process */
        void mapData( const uint32_t initDataID );

        /** Unmap per-config data to the local node process */
        void unmapData();

        /** @return the requested, default model or 0. */
        const Model* getModel( const uint32_t id );

        /** @sa eq::Config::handleEvent */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        /** @return true if the application is idling. */
        bool isIdleAA();

        /** @return true if an event required a redraw. */
        bool needsRedraw();

        /** @return true if an user event required a redraw. */
        bool isUserEvent();

    protected:
        virtual ~Config();

        /** @return a pointer to a connected admin server. */
        eq::admin::ServerPtr getAdminServer();

    private:
        int         _spinX, _spinY;
        int         _advance;
        eq::Canvas* _currentCanvas;

        LocalInitData _initData;
        FrameData     _frameData;

        Tracker _tracker;

        Models     _models;
        ModelDists _modelDist;
        eq::base::Lock  _modelLock;

        CameraAnimation _animation;

        uint64_t _messageTime;

        bool _redraw;
        bool _freeze;

        uint32_t _numFramesAA;
        eq::admin::ServerPtr _admin;

        void _loadModels();
        void _loadPath();
        void _deregisterData();
        bool _handleKeyEvent( const eq::KeyEvent& event );

        void _switchCanvas();
        void _switchView();
        void _switchModel();
        void _switchLayout( int32_t increment );

        void _setHeadMatrix( const eq::Matrix4f& matrix );
        const eq::Matrix4f& _getHeadMatrix() const;

        void _setMessage( const std::string& message );
        void _updateData();
    };
}

#endif // EQ_PLY_CONFIG_H
