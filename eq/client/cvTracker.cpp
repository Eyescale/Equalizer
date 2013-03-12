
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
#define CAPTURE_WIDTH  640
#define CAPTURE_HEIGHT 480

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
        Vector3f center( 0.f, 0.f, head_.z( ));
        Matrix3f rotation( Matrix3f::IDENTITY );

        if( eyes.size() == 2 )
        {
            const Vector2f left( face.x + eyes[0].x + eyes[0].width * .5f,
                                 face.y + eyes[0].y + eyes[0].height * .5f );
            const Vector2f right( face.x + eyes[1].x + eyes[1].width * .5f,
                                  face.y + eyes[1].y + eyes[1].height * .5f );
            center = (left + right) * .5f;

            const float distance = (right-left).length() / float(CAPTURE_WIDTH);
            center.z() = 1.f + (.16f - distance) / .16f;

            const float roll = atanf( fabs( left.y() - right.y( )) /
                                      fabs( left.x() - right.x( )) );
            rotation.array[ 0 ] = cosf( roll );
            rotation.array[ 1 ] = sinf( roll );
            rotation.array[ 4 ] = -rotation.array[ 1 ];
            rotation.array[ 5 ] =  rotation.array[ 0 ];
        }
        else
        {
            center.x() = face.x + face.width * .5f;
            center.y() = face.y + face.height * .33f; // eyes are in upper third
        }
        center = -center / Vector2f( CAPTURE_WIDTH, CAPTURE_HEIGHT ) + .5f;

        lunchbox::ScopedFastWrite mutex( lock_ );
        head_.x() = center.x();
        head_.y() = center.y();
        if( eyes.size() == 2 )
            head_.set_sub_matrix( rotation );

        LBVERB << (eyes.size() == 2 ? "" : "not ") << "using eyes, head at "
               << center << std::endl;
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
