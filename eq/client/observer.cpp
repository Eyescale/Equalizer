
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
#include <co/bufferConnection.h>

#ifdef EQUALIZER_USE_OPENCV
#  include "detail/cvTracker.h"
#endif
#ifdef EQUALIZER_USE_VRPN
#  include <vrpn_Tracker.h>
#  include <co/buffer.h>
#else
   class vrpn_Tracker_Remote;
   typedef int32_t vrpn_int32;
   typedef double vrpn_float64;
#endif

#include <limits>
#include <string>
#include <algorithm>
#include <locale>
#include <cstdlib>

#include <boost/regex.hpp>

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
class CVTracker;

class Observer
{
public:
    Observer(eq::Observer *o);

    eq::Observer *observer;

    vrpn_Tracker_Remote *vrpnTracker;
    vrpn_int32 _trackerSensorID;
    vrpn_int32 _lowestSeenSensorID;

    CVTracker* cvTracker;

#ifdef EQUALIZER_USE_VRPN
    /** Test if tracker data is for the observed sensor of interest. */
    bool canUseSensor(const vrpn_TRACKERCB &data);

    /** Callback function to call by VRPN when the tracker's position is updated. */
    static void VRPN_CALLBACK trackerCallback(void *userData, const vrpn_TRACKERCB data);
#endif

    static vrpn_int32 k_MaxSensorID;
    static vrpn_int32 k_UseLowestValidSensorID;
    static vrpn_int32 k_UseAnyValidSensorID;
};

vrpn_int32 Observer::k_MaxSensorID = std::numeric_limits<vrpn_int32>::max();
vrpn_int32 Observer::k_UseLowestValidSensorID = std::numeric_limits<vrpn_int32>::max();
vrpn_int32 Observer::k_UseAnyValidSensorID = std::numeric_limits<vrpn_int32>::max() - 1;

Observer::Observer(eq::Observer *o) :
      observer(o)
    , vrpnTracker( nullptr )
    , _trackerSensorID( k_UseLowestValidSensorID )
    , _lowestSeenSensorID( k_MaxSensorID )
    , cvTracker( nullptr )
{
}

#ifdef EQUALIZER_USE_VRPN

bool Observer::canUseSensor(const vrpn_TRACKERCB &data)
{
#ifdef DEBUG_TRACKER
    std::cout << "Eq::Observer Sensor " << data.sensor << ", position ("
              << data.pos[0] << ", " << data.pos[1] << "," << data.pos[2]
              << "), orientation (" << data.quat[0] << "," << data.quat[1]
              << "," << data.quat[2] << "," << data.quat[3] << ")" << std::endl;
#endif

    if (_trackerSensorID == k_UseLowestValidSensorID)
    {
        if( data.sensor > _lowestSeenSensorID)
            return false;

        if (data.sensor < _lowestSeenSensorID)
        {
            _lowestSeenSensorID = data.sensor;
        }
    }
    else if (_trackerSensorID != k_UseAnyValidSensorID &&
             _trackerSensorID != data.sensor)
    {
        return false;
    }

    return true;
}

void VRPN_CALLBACK Observer::trackerCallback( void* userData, const vrpn_TRACKERCB data )
{
    Observer *tracker = static_cast<Observer*>(userData);

    if (!tracker->canUseSensor(data))
        return;

    eq::Matrix4f head( eq::Matrix4f::IDENTITY );
    const vmml::quaternion<float> quat( data.quat[0], data.quat[2],
                                        -data.quat[1], data.quat[3] );
    quat.get_rotation_matrix( head );
    head.set_translation( data.pos[0], data.pos[2], -data.pos[1] );

    eq::Observer* observer = tracker->observer;
    eq::Config* config = observer->getConfig();

    MotionEvent oEvent( config );
    oEvent.command << observer->getID() << head;
    oEvent.command.disable();

    eq::ClientPtr client = config->getClient();
    co::Buffer buffer;
    buffer.swap( oEvent.buffer->getBuffer( ));

    co::ICommand iCommand( client, client, &buffer, false );
    eq::EventICommand iEvent( iCommand );
    iEvent.get< uint128_t >(); // normally done by Config::handleEvent()
    observer->handleEvent( iEvent );
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
    if( !vrpnName.empty( ))
    {
        // NOTE: loader.* EQTOKEN_STRING does not permit some symbols (e.g., ? or =), so we use - for = instead
        // <device_url> = ((<device_name>@<device_desc>)(&key-value)*
        // <device_name> = (non-space, non-@?/:;-symbols)+
        // <device_desc> = <ssep_word>@<ssep_word>[:<port>]
        static const boost::regex k_deviceURL_re("^([^\\s@:;\\?&/]+@[^\\s@;\\?&/]+)(&\\w+\\-\\w+)?$");
        static const boost::regex k_sensorid_re("^&sensorid\\-(\\w*)$");

        std::string trackerDevName;

        boost::smatch device_url;
        if (boost::regex_search(vrpnName, device_url, k_deviceURL_re))
        {
    #ifdef DEBUG_TRACKER
            for (size_t i = 0; i < device_url.size(); ++i)
            {
                std::cout << "deviceURL:" << i << " = '" << device_url.str(i) << "'" << std::endl;
            }
    #endif

            trackerDevName = device_url.str(1);

            for (size_t option_idx = 2; option_idx < device_url.size(); ++option_idx)
            {
                const std::string optionStr(device_url.str(option_idx));

                if (optionStr.empty())
                    continue;

                boost::smatch option_result;
                if (boost::regex_search(optionStr, option_result, k_sensorid_re))
                {
                    std::string optionValue(option_result.str(1));
                    std::transform(optionValue.begin(), optionValue.end(), optionValue.begin(), ::tolower);

                    if (optionValue == "any")
                        _impl->_trackerSensorID = detail::Observer::k_UseAnyValidSensorID;
                    else
                        _impl->_trackerSensorID = atoi(optionValue.c_str());
                }
                else
                    LBWARN << "VRPN tracker couldn't process option: " << optionStr;
            }
        }
        else
        {
            trackerDevName = vrpnName;
            LBWARN << "VRPN tracker couldn't process device URL: " << vrpnName;
        }


    #ifdef DEBUG_TRACKER
        std::cout << "tracker:" << trackerDevName << std::endl;
    #endif

        _impl->vrpnTracker = new vrpn_Tracker_Remote( trackerDevName.c_str( ));
        if( _impl->vrpnTracker->register_change_handler(_impl, detail::Observer::trackerCallback) != -1 )
            return true;

        LBWARN << "VRPN tracker couldn't connect to device " << vrpnName << std::endl;
        delete _impl->vrpnTracker;
        _impl->vrpnTracker = nullptr;
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

    _impl->cvTracker = new detail::CVTracker( this, camera );
    if( _impl->cvTracker->isGood( ))
        return _impl->cvTracker->start();

    delete _impl->cvTracker;
    _impl->cvTracker = 0;
    return getOpenCVCamera() == AUTO; // not a failure for auto setting
#endif
    return true;
}

bool Observer::handleEvent( EventICommand& command )
{
    switch( command.getEventType( ))
    {
    case Event::OBSERVER_MOTION:
        return setHeadMatrix( command.get< Matrix4f >( ));
    }
    return false;
}

bool Observer::configExit()
{
#ifdef EQUALIZER_USE_VRPN
    if( _impl->vrpnTracker )
    {
        _impl->vrpnTracker->unregister_change_handler( _impl, detail::Observer::trackerCallback );
        delete _impl->vrpnTracker;
        _impl->vrpnTracker = 0;
    }
#endif
#ifdef EQUALIZER_USE_OPENCV
    delete _impl->cvTracker;
    _impl->cvTracker = 0;
#endif
    return true;
}

void Observer::frameStart( const uint32_t /*frame*/ )
{
#ifdef EQUALIZER_USE_VRPN
    if( _impl->vrpnTracker )
        _impl->vrpnTracker->mainloop();
#endif
}

}

#include "../fabric/observer.ipp"
template class eq::fabric::Observer< eq::Config, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                      const eq::fabric::Observer< eq::Config, eq::Observer >& );
/** @endcond */
