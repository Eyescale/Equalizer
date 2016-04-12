
/*
 * Copyright (c) 2008-2016 Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *                         Stefan Eilemann <eile@equalizergraphics.com>
 *                         Sarah Amsellem <sarah.amsellem@gmail.com>
 *                         Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "config.h"

#include "util.h"
#include "sceneReader.h"

#include <osg/Math>

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

namespace osgScaleViewer
{
static const float maxVerticalAngle = osg::PI;
static const float minVerticalAngle = 0.2f;
static const float mouseViewSpeed = 0.005f;
static const float defaultCameraHorizontalAngle( 0.0f );
static const float defaultCameraVerticalAngle( osg::PI / 2.0f );
static const eq::Vector3f defaultCameraViewingDirection( 0., 0., -1. );

Config::Config( eq::ServerPtr parent )
    : eq::Config( parent )
    , _moveDirection( 0.0f, 0.0f, 0.0f )
    , _cameraWalkingVector( 0., 0., -1. )
    , _pointerXDiff( 0.0f )
    , _pointerYDiff( 0.0f )
    , _cameraAngleHor( defaultCameraHorizontalAngle )
    , _cameraAngleVer( defaultCameraVerticalAngle )
{
}

bool Config::init()
{
    // init tracker
    std::string trackerPort = _initData.getTrackerPort();
    if( !trackerPort.empty( ))
    {
        if( !_tracker.init( trackerPort ))
            LBWARN << "Failed to initialize tracker" << std::endl;
        else
        {
            // Set up position of tracking system wrt world space
            // Note: this depends on the physical installation.
            eq::Matrix4f matrix;
            matrix.scale( eq::Vector3f( 1.f, 1.f, -1.f ));
            _tracker.setWorldToEmitter( matrix );

            matrix = eq::Matrix4f();
            matrix.rotate_z( -M_PI_2 );
            _tracker.setSensorToObject( matrix );
            LBINFO << "Tracker initialized" << std::endl;
        }
    }

    registerObject( &_frameData );
    _initData.setFrameDataID( _frameData.getID( ));
    registerObject( &_initData );

    return eq::Config::init( _initData.getID( ));
}

bool Config::exit()
{
    bool ret = eq::Config::exit();

    _initData.setFrameDataID( eq::uint128_t( ));
    deregisterObject( &_initData );
    deregisterObject( &_frameData );
    return ret;
}

void Config::updateFrameData( float elapsed )
{
    // Update the viewing direction based on the mouse movement
    _cameraAngleHor += mouseViewSpeed * _pointerXDiff;
    if( _cameraAngleHor > 2 * osg::PI || _cameraAngleHor < -2 * osg::PI )
        _cameraAngleHor = 0.0f;
    _cameraAngleVer += mouseViewSpeed * _pointerYDiff;
    if( _cameraAngleVer > maxVerticalAngle )
        _cameraAngleVer = maxVerticalAngle;
    if( _cameraAngleVer < minVerticalAngle )
        _cameraAngleVer = minVerticalAngle;
    _pointerXDiff = _pointerYDiff = 0.0f;

    eq::Vector3f cameraViewingDirection;
    cameraViewingDirection.x() = sin( _cameraAngleHor ) * sin( _cameraAngleVer );
    cameraViewingDirection.z() = -cos( _cameraAngleHor ) * sin( _cameraAngleVer );
    cameraViewingDirection.y() = cos( _cameraAngleVer );
    _cameraWalkingVector = cameraViewingDirection;
    _cameraWalkingVector.y() = 0.0f;

    // save camera data to frame data and update the camera position
    _frameData.setCameraPosition( _frameData.getCameraPosition() +
                                  _moveDirection * elapsed );
    _frameData.setCameraLookAtPoint( _frameData.getCameraPosition() +
                                     cameraViewingDirection );
    _frameData.setCameraUpVector( eq::Vector3f( 0., 1., 0. ));
}

uint32_t Config::startFrame()
{
    // update head position
    if( _tracker.isRunning( ))
    {
        _tracker.update();
        const eq::Matrix4f& headMatrix = _tracker.getMatrix();
        _setHeadMatrix( headMatrix );
    }

    float elapsed = _clock.getTimef();
    _clock.reset();
    updateFrameData( elapsed );

    const eq::uint128_t version = _frameData.commit();
    return eq::Config::startFrame( version );
}

void Config::setInitData( const InitData& data )
{
    _initData = data;
}

const InitData& Config::getInitData() const
{
    return _initData;
}

bool Config::loadInitData( const eq::uint128_t& id )
{
    LBASSERT( !_initData.isAttached( ));
    return getClient()->syncObject( &_initData, getApplicationNode(), id );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    const float moveSpeed = .1f;

    switch( event->data.type )
    {
        // set mMoveDirection to a null vector after a key is released
        // so that the updating of the camera position stops
        case eq::Event::KEY_RELEASE:
            if ( event->data.keyPress.key >= 261 &&
                 event->data.keyPress.key <= 266 )
                _moveDirection = eq::Vector3f( 0, 0, 0 );
        break;

        // change mMoveDirection when the appropriate key is pressed
        case eq::Event::KEY_PRESS:
            switch ( event->data.keyPress.key )
            {
                case eq::KC_LEFT:
                    _moveDirection = vmml::normalize( orthographicVector(
                                     _cameraWalkingVector )) * moveSpeed;
                    return true;

                case eq::KC_UP:
                    _moveDirection.y() = moveSpeed;
                    return true;

                case eq::KC_RIGHT:
                    _moveDirection = -vmml::normalize( orthographicVector(
                                     _cameraWalkingVector )) * moveSpeed;
                    return true;

                case eq::KC_DOWN:
                    _moveDirection.y() = -moveSpeed;
                    return true;

                case eq::KC_PAGE_UP:
                    _moveDirection = vmml::normalize( _cameraWalkingVector )
                                     * moveSpeed;
                    return true;

                case eq::KC_PAGE_DOWN:
                    _moveDirection = -vmml::normalize( _cameraWalkingVector ) *
                                      moveSpeed;
                    return true;

                case 's':
                    _frameData.toggleStatistics();
            }
            break;

        // turn left and right, up and down with mouse pointer
        case eq::Event::CHANNEL_POINTER_MOTION:
            if ( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 &&
                 event->data.pointerMotion.x <= event->data.context.pvp.w &&
                 event->data.pointerMotion.x >= 0 &&
                 event->data.pointerMotion.y <= event->data.context.pvp.h &&
                 event->data.pointerMotion.y >= 0 )
            {
                _pointerXDiff += event->data.pointerMotion.dx;
                _pointerYDiff += event->data.pointerMotion.dy;
                return true;
            }
            break;
    }

    // let Equalizer handle any events we don't handle ourselves here, like the
    // escape key for closing the application.
    return eq::Config::handleEvent( event );
}

// Note: real applications would use one tracking device per observer
void Config::_setHeadMatrix( const eq::Matrix4f& matrix )
{
    const eq::Observers& observers = getObservers();
    for( eq::Observers::const_iterator i = observers.begin();
         i != observers.end(); ++i )
    {
        (*i)->setHeadMatrix( matrix );
    }
}

const eq::Matrix4f& Config::_getHeadMatrix() const
{
    const eq::Observers& observers = getObservers();
    static const eq::Matrix4f identity;
    if( observers.empty( ))
        return identity;

    return observers[0]->getHeadMatrix();
}
}
