
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

#include <eq/eq.h>

#include "localInitData.h" // member
#include "frameData.h"     // member
#include "tracker.h"       // member

namespace eqPly
{
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
        bool mapData( const uint32_t initDataID );

        /** @return the loaded model, or 0. */
        const Model* getModel() const { return _model; }

    protected:
        virtual ~Config();

        /** @sa eq::Config::handleEvent */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        int        _spinX, _spinY;

        LocalInitData _initData;
        FrameData     _frameData;

        Tracker _tracker;

        Model*     _model;
        ModelDist* _modelDist;

    private:
        void _loadModel();
    };
}

#endif // EQ_PLY_CONFIG_H
