
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

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
        const Model* getModel( const uint32_t id = EQ_ID_INVALID );

        /** @sa eq::Config::handleEvent */
        virtual bool handleEvent( const eq::ConfigEvent* event );

        /** @return true if an event required a redraw. */
        bool needsRedraw();

    protected:
        virtual ~Config();

        int        _spinX, _spinY;

        LocalInitData _initData;
        FrameData     _frameData;

        Tracker _tracker;

        ModelVector     _models;
        ModelDistVector _modelDist;
        eq::base::SpinLock _modelLock;

        bool _redraw;

    private:
        void _loadModels();
    };
}

#endif // EQ_PLY_CONFIG_H
