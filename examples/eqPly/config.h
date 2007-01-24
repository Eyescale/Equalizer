
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

#include <eq/eq.h>

#include "appInitData.h"
#include "frameData.h"

class Config : public eq::Config
{
public:
    Config();

    bool isRunning() const { return _running; }
    
    /** @sa eq::Config::init. */
    virtual bool init();
    /** @sa eq::Config::exit. */
    virtual bool exit();

    /** @sa eq::Config::startFrame. */
    virtual uint32_t startFrame();

    eqBase::RefPtr<AppInitData> getInitData() { return _initData; }

protected:
    virtual ~Config();

    eqNet::Object* instanciateObject( const uint32_t type, const void* data, 
                                       const uint64_t dataSize )
        {
            switch( type )
            {
                case TYPE_INITDATA:
                    return new InitData( data, dataSize );
                case TYPE_FRAMEDATA:
                    return new FrameData( data, dataSize );
                default:
                    return eq::Config::instanciateObject( type, data,
                                                          dataSize );
            }
        }

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( const eq::ConfigEvent* event );

    bool       _running;
    int        _spinX, _spinY;

    eqBase::RefPtr<AppInitData> _initData;
    eqBase::RefPtr<FrameData>   _frameData;

private:
    static void _applyRotation( float m[16], const float dx, const float dy );
};

#endif // EQ_PLY_CONFIG_H
