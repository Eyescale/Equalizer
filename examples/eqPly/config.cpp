
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

Config::Config()
        : _running( false ),
          _spinX( 5 ),
          _spinY( 5 )
{
    _initData  = new AppInitData;
    _frameData = new FrameData;
}

Config::~Config()
{
    _initData  = 0;
    _frameData = 0;
}

bool Config::init()
{
    registerObject( _initData.get(), getLocalNode( ));
    registerObject( _frameData.get(), getLocalNode( ));

    _initData->setFrameData( _frameData );

    _running = eq::Config::init( _initData->getID( ));
    return _running;
}

bool Config::exit()
{
    _running = false;
    const bool ret = eq::Config::exit();

    _initData->setFrameData( 0 );
    deregisterObject( _frameData.get( ));
    deregisterObject( _initData.get( ));

    EQASSERT( _frameData->getRefCount() == 1 );
    EQASSERT( _initData->getRefCount() == 1 );
    return ret;
}

uint32_t Config::beginFrame()
{
    // update database
    _frameData->_data.rotation.preRotateX( -0.001 * _spinX );
    _frameData->_data.rotation.preRotateY( -0.001 * _spinY );
    const uint32_t version = _frameData->commit();

    return eq::Config::beginFrame( version );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->type )
    {
        case eq::ConfigEvent::TYPE_KEY_PRESS:
            switch( event->keyPress.key )
            {
                case 'q':
                case eq::KC_ESCAPE:
                    _running = false;
                    return true;

                case 'r':
                case ' ':
                    _spinX = 0;
                    _spinY = 0;
                    _frameData->reset();
                    return true;
            }
            break;

        case eq::ConfigEvent::TYPE_POINTER_BUTTON_PRESS:
            if( event->pointerButtonPress.buttons == 
                ( eq::PTR_BUTTON1 | eq::PTR_BUTTON2 | eq::PTR_BUTTON3 ))
            {
                _running = false;
                return true;
            }
            break;

        case eq::ConfigEvent::TYPE_POINTER_BUTTON_RELEASE:
            if( event->pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE &&
                event->pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinX = event->pointerButtonRelease.dx;
                _spinY = event->pointerButtonRelease.dy;
            }
            return true;

        case eq::ConfigEvent::TYPE_POINTER_MOTION:
            if( event->pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                _frameData->_data.rotation.preRotateX( 
                    -0.005*event->pointerMotion.dx );
                _frameData->_data.rotation.preRotateY(
                    -0.005*event->pointerMotion.dy );
            }
            else if( event->pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->pointerMotion.buttons == ( eq::PTR_BUTTON1 |
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData->_data.translation.z +=
                    .005 * event->pointerMotion.dy;
            }
            else if( event->pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData->_data.translation.x += 
                    .0005 * event->pointerMotion.dx;
                _frameData->_data.translation.y -= 
                    .0005 * event->pointerMotion.dy;
            }
            return true;

        default:
            break;
    }
    return eq::Config::handleEvent( event );
}
