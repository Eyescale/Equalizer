
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQPLY_CONFIG_H
#define EQPLY_CONFIG_H

#include <eq/eq.h>

#include "localInitData.h" // member
#include "frameData.h"     // member
#include "tracker.h"       // member

namespace eqPly
{
    typedef std::vector< Model* >     ModelVector;
    typedef std::vector< ModelDist* > ModelDistVector;

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

        /** @return true if an event required a redraw. */
        bool needsRedraw();

    protected:
        virtual ~Config();

        int         _spinX, _spinY;
        int         _advance;
        eq::Canvas* _currentCanvas;

        LocalInitData _initData;
        FrameData     _frameData;

        Tracker _tracker;

        struct Step
        {
            Step( int frame_, const vmml::Vector3f& translation_,
                              const vmml::Vector3f& rotation_  )
                : frame( frame_ ), translation( translation_ ),
                  rotation( rotation_ ){};

            int frame;
            vmml::Vector3f translation;
            vmml::Vector3f rotation;
        };
        struct Path
        {
            Path() : _curStep( 0 ), _curFrame( 0 ) {}
            bool valid() const { return !_steps.empty(); }
            void addStep( const Step& step ) { _steps.push_back( step ); }
            const Step getNextStep();

            vmml::Vector3f modelRotation;

        private:
            std::vector< Step > _steps;
            uint32_t _curStep;
            int  _curFrame;
        } _path;

        ModelVector     _models;
        ModelDistVector _modelDist;
        eq::base::SpinLock _modelLock;

        bool _redraw;

    private:
        void _loadModels();
        void _loadPath();
        void _deregisterData();
        bool _handleKeyEvent( const eq::KeyEvent& event );
        void _setHeadMatrix( const vmml::Matrix4f& matrix );
        const vmml::Matrix4f& _getHeadMatrix() const;
    };
}

#endif // EQPLY_CONFIG_H
