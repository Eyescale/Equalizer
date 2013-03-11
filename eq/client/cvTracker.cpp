
/* Copyright (c) 2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "cvTracker.h"

#define FACE_CONFIG std::string( OPENCV_INSTALL_PATH ) +            \
    "/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"
#define EYE_CONFIG  std::string( OPENCV_INSTALL_PATH ) +                \
    "/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml"
#define CAPTURE_WIDTH  320
#define CAPTURE_HEIGHT 240

namespace eq
{
namespace detail
{
CVTracker::CVTracker( const uint32_t camera )
    : camera_( camera )
    , capture_( cvCaptureFromCAM( camera_ ))
    , head_( Matrix4f::IDENTITY )
    , running_( false )
{
	if( !capture_ )
    {
        EQWARN << "Found no OpenCV camera " << camera_ << std::endl;
        return;
    }

    if( !faceDetector_.load( FACE_CONFIG ))
    {
        EQWARN << "Can't set up face detector using " << FACE_CONFIG
               << std::endl;

        cvReleaseCapture( &capture_ );
        capture_ = 0;
        return;
    }

    if( !eyeDetector_.load( EYE_CONFIG ))
    {
        EQWARN << "Can't set up eye detector using " << EYE_CONFIG << std::endl;

        cvReleaseCapture( &capture_ );
        capture_ = 0;
        return;
    }

    cvSetCaptureProperty( capture_, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_WIDTH );
    cvSetCaptureProperty( capture_, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_HEIGHT );
    LBINFO << "Activated tracking camera " << camera_ << std::endl;
}

CVTracker::~CVTracker()
{
    running_ = false;
    join();
    cvReleaseCapture( &capture_ );
    capture_ = 0;
}

void CVTracker::run()
{
    running_ = true;
    while( running_ )
    {
        const cv::Mat frame = cvQueryFrame( capture_ );
        if( frame.empty( ))
        {
            EQWARN << "Failure to grab a video frame, bailing" << std::endl;
            break;
        }

        cv::Mat bwFrame;
        cvtColor( frame, bwFrame, CV_BGR2GRAY );
        equalizeHist( bwFrame, bwFrame );

        // detect face
        std::vector< cv::Rect > faces;
        faceDetector_.detectMultiScale( bwFrame, faces, 1.1f, 2,
                                        CV_HAAR_SCALE_IMAGE |
                                        CV_HAAR_FIND_BIGGEST_OBJECT,
                                        cv::Size( 30, 30 ));
        if( faces.empty( ))
            continue;

        // detect eyes
        const cv::Rect& face = faces.front();
        const cv::Mat faceROI = bwFrame( face );
        std::vector< cv::Rect > eyes;
        eyeDetector_.detectMultiScale( faceROI, eyes, 1.1f, 2,
                                       CV_HAAR_SCALE_IMAGE, cv::Size( 15, 15 ));
        if( eyes.size() == 2 )
        {
            // TODO roll and depth estimation using eyes
            LBINFO << "Eyes " << eyes[0] << ", " << eyes[1] << std::endl;
        }
        else
        {
            lunchbox::ScopedFastWrite mutex( lock_ );
            head_.x() = .5f - ( face.x + face.width * .5f ) /
                float(CAPTURE_WIDTH);
            head_.y() = .5f - ( face.y + face.height * .5f ) /
                float(CAPTURE_HEIGHT);
            LBVERB << eyes.size() << " eye detected, head at (" << head_.x()
                   << ", " << head_.y() << ", " << head_.z() << ")" <<std::endl;
        }
    }

    running_ = false;
}

Matrix4f CVTracker::getHeadMatrix() const
{
    lunchbox::ScopedFastRead mutex( lock_ );
    return head_;
}

}
}
