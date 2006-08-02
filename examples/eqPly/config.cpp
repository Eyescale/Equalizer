
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"
#include <math.h>

Config::Config()
        : _running( false ),
          _frameData(NULL),
          _spinX(0),
          _spinY(0)
{}

bool Config::init( const uint32_t initID )
{
    _running = eq::Config::init( initID );
    return _running;
}

uint32_t Config::beginFrame()
{
    // update database
    _applyRotation(_frameData->_data.rotation, 0.001 * _spinX, 0.001 * _spinY );
    const uint32_t version = _frameData->commit();

    return eq::Config::beginFrame( version );
}

bool Config::handleEvent( eq::ConfigEvent* event )
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

        case eq::ConfigEvent::TYPE_POINTER_BUTTON_RELEASE:
            if( event->pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE &&
                event->pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinX = -event->pointerButtonRelease.dx;
                _spinY = -event->pointerButtonRelease.dy;
            }
            return true;

        case eq::ConfigEvent::TYPE_POINTER_MOTION:
            if( event->pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;
                
                _applyRotation(_frameData->_data.rotation, -0.005*event->pointerMotion.dx, -0.005*event->pointerMotion.dy );
            }
            else if( event->pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->pointerMotion.buttons == ( eq::PTR_BUTTON1 | 
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData->_data.translation[2] +=
                    .005 * event->pointerMotion.dy;
            }
            else if( event->pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData->_data.translation[0] += 
                    .0005 * event->pointerMotion.dx;
                _frameData->_data.translation[1] -= 
                    .0005 * event->pointerMotion.dy;
            }
            return true;

        default:
            break;
    }
    return eq::Config::handleEvent( event );
}

void Config::_applyRotation( float m[16], const float dx, const float dy )
{
    //rotation matrix
    float rotxy[16] = { cos(dx),            0,       sin(dx),          0,
                        sin(dy) * sin(dx),  cos(dy), -sin(dy)*cos(dx), 0,
                        cos(dy)*(-sin(dx)), sin(dy), cos(dy)*cos(dx),  0,
                        0,                  0,       0,                1};

    float rotM[16];
    memcpy( rotM, m, 16*sizeof( float ));
    bzero( m, 16*sizeof( float ));

    //_data.rotation = rotM * rotxy           matrix multiplication
    for( int i=0; i<4; i++ )
    {
        for( int k=0; k<4; k++ )
        {
            for( int n=0; n<4; n++ )
            {
                m[4*i+k] += rotM[4*i+n] * rotxy[n*4+k];
            }
        }
    }
}
