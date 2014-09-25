
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "observer.h"

#include "config.h"
#include "client.h"
#include "event.h"
#include "eventICommand.h"
#include "server.h"

#include <eq/fabric/paths.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/vrpnTracker.h>
#include <co/bufferConnection.h>

#ifdef EQUALIZER_USE_OPENCV
#  include "detail/cvTracker.h"
#endif
#ifdef EQUALIZER_USE_VRPN
#  include <vrpn_Tracker.h>
#  include <co/buffer.h>
#endif

#include <string>

#include <boost/scoped_ptr.hpp>

//#define DEBUG_TRACKER

namespace eq
{

#ifdef EQUALIZER_USE_VRPN
namespace
{
class MotionEvent
{
public:
    MotionEvent( const co::Object* object )
        : buffer( new co::BufferConnection )
        , command( co::Connections( 1, buffer ), fabric::CMD_CONFIG_EVENT,
                   co::COMMANDTYPE_OBJECT, object->getID(),
                   object->getInstanceID( ))
    {
        command << Event::OBSERVER_MOTION;
    }

    co::BufferConnectionPtr buffer;
    EventOCommand command;
};
}
#endif

namespace detail
{
class Observer
{
public:
    Observer(eq::Observer *o);

    eq::Observer *observer;

#ifdef EQUALIZER_USE_VRPN
    boost::scoped_ptr<vrpn_Tracker_Remote> vrpnTracker;
    eq::fabric::Matrix3f _axisTransform;
    int32_t _sensorID;
    int32_t _lowestSeenSensorID;
#endif

#ifdef EQUALIZER_USE_OPENCV
    boost::scoped_ptr<CVTracker> cvTracker;
#endif

#ifdef EQUALIZER_USE_VRPN
    /** Test if tracker data is for the observed sensor of interest. */
    bool canUseSensor(const vrpn_TRACKERCB &data);

    /** VRPN callback function when the tracker's position is updated. */
    static void VRPN_CALLBACK trackerCallback(void *userData,
                                              const vrpn_TRACKERCB data);
#endif

    static int32_t k_MaxSensorID;
};

int32_t Observer::k_MaxSensorID = std::numeric_limits<int32_t>::max();

Observer::Observer(eq::Observer *o) :
      observer(o)
#ifdef EQUALIZER_USE_VRPN
    , vrpnTracker()
    , _axisTransform(eq::fabric::Matrix3f::IDENTITY)
    , _sensorID( eq::fabric::VRPNTrackerSensor::UseLowestValidSensorID )
    , _lowestSeenSensorID( k_MaxSensorID )
#endif
#ifdef EQUALIZER_USE_OPENCV
    , cvTracker()
#endif
{
}

#ifdef EQUALIZER_USE_VRPN

bool Observer::canUseSensor(const vrpn_TRACKERCB &data)
{
#ifdef DEBUG_TRACKER
    std::cout << "Eq::Observer Sensor " << data.sensor << ", position ("
              << data.pos[0] << ", " << data.pos[1] << "," << data.pos[2]
              << "), orientation (" << data.quat[0] << "," << data.quat[1]
              << "," << data.quat[2] << "," << data.quat[3] << ")" <<
                 std::endl;
#endif

    if (_sensorID == eq::fabric::VRPNTrackerSensor::UseLowestValidSensorID)
    {
        if( data.sensor > _lowestSeenSensorID)
            return false;

        if (data.sensor < _lowestSeenSensorID)
        {
            _lowestSeenSensorID = data.sensor;
        }
    }
    else if (_sensorID != eq::fabric::VRPNTrackerSensor::UseAnyValidSensorID &&
             _sensorID != data.sensor)
    {
        return false;
    }

    return true;
}

void VRPN_CALLBACK Observer::trackerCallback( void* userData,
                                              const vrpn_TRACKERCB data )
{
    Observer *tracker = static_cast<Observer*>(userData);

    if (!tracker->canUseSensor(data))
        return;

    using namespace eq::fabric;
    const Matrix3f& t(tracker->_axisTransform);
    const Vector3f tQ = t * Vector3f( data.quat[0], data.quat[1], data.quat[2] );
    const Vector3f tP = t * Vector3f( data.pos[0], data.pos[1], data.pos[2] );

    eq::Matrix4f head( eq::Matrix4f::IDENTITY );
    const vmml::quaternion<float> quat( tQ[0], tQ[1], tQ[2], data.quat[3] );

    quat.get_rotation_matrix( head );
    head.set_translation( tP );

    eq::Observer* observer = tracker->observer;
    eq::Config* config = observer->getConfig();

    // Directly dispatch the event: We're called from Config::startFrame and
    // need to process now without sending the event so that the change is
    // committed and takes effect for this frame.
    MotionEvent oEvent( config );
    oEvent.command << observer->getID() << head;
    oEvent.command.disable();

    eq::ClientPtr client = config->getClient();
    co::Buffer buffer;
    buffer.swap( oEvent.buffer->getBuffer( ));

    co::ICommand iCommand( client, client, &buffer, false );
    eq::EventICommand iEvent( iCommand );
    config->handleEvent( iEvent ); // config dispatch so app can update state
}
#endif
}

typedef fabric::Observer< Config, Observer > Super;

Observer::Observer( Config* parent )
        : Super( parent )
        , _impl( new detail::Observer(this) )
{}

Observer::~Observer()
{
    delete _impl;
}

ServerPtr Observer::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

bool Observer::configInit()
{
#ifdef EQUALIZER_USE_VRPN
    const std::string& vrpnName = getVRPNTracker();
    if( !vrpnName.empty() )
    {
        using namespace eq::fabric;

        _impl->_sensorID = getVRPNTrackerSensor();
        _impl->_axisTransform = getVRPNTrackerAxis().getTransform();

        if (_impl->_sensorID < 0 &&
                _impl->_sensorID != VRPNTrackerSensor::UseAnyValidSensorID &&
                _impl->_sensorID != VRPNTrackerSensor::UseLowestValidSensorID)
        {
            LBWARN << "VRPN tracker unknown sensor option: "
                   << _impl->_sensorID;
            _impl->_sensorID = VRPNTrackerSensor::UseLowestValidSensorID;
        }

#ifdef DEBUG_TRACKER
        std::cout << "tracker:" << vrpnName
                  << " sensor:" << _impl->_sensorID
                  << " axis: " << getVRPNTrackerAxis() << " ("
                  << _impl->_axisTransform << ")"
                  << std::endl;
#endif

        _impl->vrpnTracker.reset(
                    new vrpn_Tracker_Remote( vrpnName.c_str() ));

        if( _impl->vrpnTracker->connectionPtr() &&
            _impl->vrpnTracker->register_change_handler(_impl,
                    eq::detail::Observer::trackerCallback) != -1 )
            return true;

        LBWARN << "VRPN tracker couldn't connect to device " << vrpnName <<
                                                                std::endl;
        _impl->vrpnTracker.reset();
        return false;
    }
#endif
#ifdef EQUALIZER_USE_OPENCV
    int32_t camera = getOpenCVCamera();
    if( camera == OFF )
        return true;
    if( camera == AUTO )
        camera = getPath().observerIndex;
    else
        --camera; // .eqc counts from 1, OpenCV from 0

    _impl->cvTracker.reset(new detail::CVTracker( this, camera ));
    if( _impl->cvTracker->isGood( ))
        return _impl->cvTracker->start();

    _impl->cvTracker.reset();
    return getOpenCVCamera() == AUTO; // not a failure for auto setting
#endif
    return true;
}

bool Observer::handleEvent( EventICommand& command )
{
    switch( command.getEventType( ))
    {
    case Event::OBSERVER_MOTION:
        return setHeadMatrix( command.read< Matrix4f >( ));
    }
    return false;
}

bool Observer::configExit()
{
#ifdef EQUALIZER_USE_VRPN
    if( _impl->vrpnTracker.get() )
    {
        _impl->vrpnTracker->unregister_change_handler( _impl,
                detail::Observer::trackerCallback );
        _impl->vrpnTracker.reset();
    }
#endif
#ifdef EQUALIZER_USE_OPENCV
    _impl->cvTracker.reset();
#endif
    return true;
}

void Observer::frameStart( const uint32_t /*frame*/ )
{
#ifdef EQUALIZER_USE_VRPN
    if( _impl->vrpnTracker.get() )
        _impl->vrpnTracker->mainloop();
#endif
}

}

#include "../fabric/observer.ipp"
template class eq::fabric::Observer< eq::Config, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << (
                            std::ostream&,
                            const eq::fabric::Observer< eq::Config,
                            eq::Observer >& );
/** @endcond */
