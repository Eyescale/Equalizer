
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

#  define FACE_CONFIG std::string( OPENCV_INSTALL_PATH ) + \
    "/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"
#  define EYE_CONFIG  std::string( OPENCV_INSTALL_PATH ) + \
    "/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml"
#  define CAPTURE_WIDTH  320
#  define CAPTURE_HEIGHT 240
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
    cv::CascadeClassifier faceDetector;
    cv::CascadeClassifier eyeDetector;
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

    if( !impl_->faceDetector.load( FACE_CONFIG ))
    {
        EQWARN << "Can't set up face detector using " << FACE_CONFIG
               << std::endl;

        cvReleaseCapture( &impl_->capture );
        impl_->capture = 0;
        return getOpenCVCamera() == AUTO; // not a failure for auto setting
    }

    if( !impl_->eyeDetector.load( EYE_CONFIG ))
    {
        EQWARN << "Can't set up eye detector using " << EYE_CONFIG << std::endl;

        cvReleaseCapture( &impl_->capture );
        impl_->capture = 0;
        return getOpenCVCamera() == AUTO; // not a failure for auto setting
    }

    cvSetCaptureProperty( impl_->capture, CV_CAP_PROP_FRAME_WIDTH,
                          CAPTURE_WIDTH );
    cvSetCaptureProperty( impl_->capture, CV_CAP_PROP_FRAME_HEIGHT,
                          CAPTURE_HEIGHT );
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

    cv::Mat frame = cvQueryFrame( impl_->capture );
    if( frame.empty( ))
        return;

    cv::Mat bwFrame;
    cvtColor( frame, bwFrame, CV_BGR2GRAY );
    equalizeHist( bwFrame, bwFrame );

    std::vector< cv::Rect > faces;
    impl_->faceDetector.detectMultiScale( bwFrame, faces, 1.1f, 2,
                                          CV_HAAR_SCALE_IMAGE |
                                          CV_HAAR_FIND_BIGGEST_OBJECT,
                                          cv::Size( 30, 30 ));
    if( faces.empty( ))
        return;

    // detect eyes
    const cv::Rect& face = faces.front();
    const cv::Mat faceROI = bwFrame( face );
    std::vector< cv::Rect > eyes;
    impl_->eyeDetector.detectMultiScale( faceROI, eyes, 1.1f, 2,
                                         CV_HAAR_SCALE_IMAGE,
                                         cv::Size( 30, 30 ));
    if( eyes.size() < 2 )
    {
        Matrix4f head = getHeadMatrix();
        head.x() = .5f - ( face.x + face.width * .5f ) / float(CAPTURE_WIDTH);
        head.y() = .5f - ( face.y + face.height * .5f ) / float(CAPTURE_HEIGHT);
        setHeadMatrix( head );

        LBVERB << eyes.size() << " eye detected, head at (" << head.x() << ", "
               << head.y() << ")" << std::endl;
        return;
    }

    LBVERB << "Eyes " << eyes[0] << ", " << eyes[1] << std::endl;
#endif
}

}

#include "../fabric/observer.ipp"
template class eq::fabric::Observer< eq::Config, eq::Observer >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                      const eq::fabric::Observer< eq::Config, eq::Observer >& );
/** @endcond */
