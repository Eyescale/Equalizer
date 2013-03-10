
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
#include "server.h"

#include <eq/fabric/paths.h>

#ifdef EQ_USE_OPENCV
#  include <opencv2/opencv.hpp>
#endif

namespace eq
{
namespace detail
{
class Observer
{
public:
    Observer()
#ifdef EQ_USE_OPENCV
        : capture( 0 )
#endif
    {}

#ifdef EQ_USE_OPENCV
    CvCapture* capture;
#endif
};
}

typedef fabric::Observer< Config, Observer > Super;

Observer::Observer( Config* parent )
        : Super( parent )
        , impl_( new detail::Observer )
{
}

Observer::~Observer()
{
    delete impl_;
}

ServerPtr Observer::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

bool Observer::configInit()
{
#ifdef EQ_USE_OPENCV
    int32_t camera = getOpenCVCamera();
    if( camera == OFF )
        return true;
    if( camera == AUTO )
        camera = getPath().observerIndex;
    else
        --camera; // .eqc counts from 1, OpenCV from 0

    impl_->capture = cvCaptureFromCAM( camera );
	if( !impl_->capture )
    {
        EQWARN << "Found no OpenCV camera " << camera << " for " << *this
               << std::endl;
        return getOpenCVCamera() == AUTO; // not a failure for auto setting
    }

    cvSetCaptureProperty( impl_->capture, CV_CAP_PROP_FRAME_WIDTH, 320 );
    cvSetCaptureProperty( impl_->capture, CV_CAP_PROP_FRAME_HEIGHT, 240 );
    LBINFO << "Activated tracking camera " << camera << " for " << *this
           << std::endl;
#endif
    return true;
}

bool Observer::configExit()
{
#ifdef EQ_USE_OPENCV
	if( !impl_->capture )
        cvReleaseCapture( &impl_->capture );
    impl_->capture = 0;
#endif
    return true;
}

void Observer::frameStart( const uint32_t frameNumber )
{
#ifdef EQ_USE_OPENCV
    if( !impl_->capture )
        return;
#endif
}

}

#include "../fabric/observer.ipp"
template class eq::fabric::Observer< eq::Config, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                      const eq::fabric::Observer< eq::Config, eq::Observer >& );
/** @endcond */
