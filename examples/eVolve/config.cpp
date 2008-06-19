
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

using namespace std;

namespace eVolve
{

Config::Config( eqBase::RefPtr< eq::Server > parent )
        : eq::Config( parent )
        , _spinX( 5 )
        , _spinY( 5 )
{
}

Config::~Config()
{
}

bool Config::init()
{
    // init distributed objects
    _frameData.data.ortho = _initData.getOrtho();
    registerObject( &_frameData );
    _initData.setFrameDataID( _frameData.getID( ));

    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
        return false;

    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit();

    _initData.setFrameDataID( EQ_ID_INVALID );
    deregisterObject( &_initData );
    deregisterObject( &_frameData );

    return ret;
}

uint32_t Config::startFrame()
{
    // update database
    _frameData.data.rotation.preRotateX( -0.001f * _spinX );
    _frameData.data.rotation.preRotateY( -0.001f * _spinY );
    const uint32_t version = _frameData.commit();

    return eq::Config::startFrame( version );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
            switch( event->data.keyPress.key )
            {
                case 'r':
                case 'R':
                case ' ':
                    _spinX = 0;
                    _spinY = 0;
                    _frameData.reset();
                    return true;

                case 'o':
                case 'O':
                    _frameData.data.ortho = !_frameData.data.ortho;
                    return true;

                default:
                    break;
            }
            break;

        case eq::Event::POINTER_BUTTON_RELEASE:
            if( event->data.pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE
                && event->data.pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinX = event->data.pointerButtonRelease.dx;
                _spinY = event->data.pointerButtonRelease.dy;
            }
            return true;

        case eq::Event::POINTER_MOTION:
            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                _frameData.data.rotation.preRotateX( 
                    -0.005f * event->data.pointerMotion.dx );
                _frameData.data.rotation.preRotateY(
                    -0.005f * event->data.pointerMotion.dy );
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->data.pointerMotion.buttons == ( eq::PTR_BUTTON1 |
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData.data.translation.z +=
                    .005f * event->data.pointerMotion.dy;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData.data.translation.x += 
                    .0005f * event->data.pointerMotion.dx;
                _frameData.data.translation.y -= 
                    .0005f * event->data.pointerMotion.dy;
            }
            return true;

        default:
            break;
    }
    return eq::Config::handleEvent( event );
}
}
