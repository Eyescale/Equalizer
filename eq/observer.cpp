
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "eventICommand.h"
#include "server.h"

#include <eq/fabric/event.h>
#include <eq/fabric/paths.h>
#include <eq/fabric/commands.h>
#include <co/bufferConnection.h>
#include <vmmlib/quaternion.hpp>

#ifdef EQUALIZER_USE_OPENCV
#  include "detail/cvTracker.h"
#endif
#ifdef EQUALIZER_USE_VRPN
#  include <vrpn_Tracker.h>
#  include <co/buffer.h>
#else
   class vrpn_Tracker_Remote;
#endif


namespace eq
{
namespace detail
{
class CVTracker;

class Observer
{
public:
    Observer()
        : vrpnTracker( 0 )
        , cvTracker( 0 )
    {}

    vrpn_Tracker_Remote *vrpnTracker;
    CVTracker* cvTracker;
};
}

typedef fabric::Observer< Config, Observer > Super;

Observer::Observer( Config* parent )
        : Super( parent )
        , _impl( new detail::Observer )
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

void VRPN_CALLBACK trackerCB( void* userdata, const vrpn_TRACKERCB data )
{
    if( data.sensor != 0 )
        return; // Only use first sensor

    const Matrix4f head( Quaternionf( data.quat[0], data.quat[1],
                                      data.quat[2], data.quat[3] ),
                         Vector3f( data.pos[0], data.pos[1], data.pos[2] ));
    Observer *observer = static_cast< Observer* >( userdata );
    Config* config = observer->getConfig();

    // Directly dispatch the event: We're called from Config::startFrame and
    // need to process now without sending the event so that the change is
    // committed and takes effect for this frame.
    MotionEvent oEvent( config );
    oEvent.command << observer->getID() << head;
    oEvent.command.disable();

    ClientPtr client = config->getClient();
    co::Buffer buffer;
    buffer.swap( oEvent.buffer->getBuffer( ));

    co::ICommand iCommand( client, client, &buffer, false );
    EventICommand iEvent( iCommand );
    config->handleEvent( iEvent ); // config dispatch so app can update state
}
}
#endif

bool Observer::configInit()
{
#ifdef EQUALIZER_USE_VRPN
    const std::string& vrpnName = getVRPNTracker();
    if( !vrpnName.empty( ))
    {
        _impl->vrpnTracker = new vrpn_Tracker_Remote( vrpnName.c_str( ));
        if( _impl->vrpnTracker->register_change_handler(this, trackerCB) != -1 )
            return true;

        LBWARN << "VRPN tracker couldn't connect to device " << vrpnName
               << std::endl;
        delete _impl->vrpnTracker;
        _impl->vrpnTracker = 0;
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
        return setHeadMatrix( command.read< Matrix4f >( ));
    }
    return false;
}

bool Observer::configExit()
{
#ifdef EQUALIZER_USE_VRPN
    if( _impl->vrpnTracker )
    {
        _impl->vrpnTracker->unregister_change_handler( this, trackerCB );
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

#include <eq/fabric/observer.ipp>
template class eq::fabric::Observer< eq::Config, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                      const eq::fabric::Observer< eq::Config, eq::Observer >& );
/** @endcond */
