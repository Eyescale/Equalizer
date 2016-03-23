
/* Copyright (c) 2013-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "../config.h"
#include "../observer.h"

#include <eq/fabric/event.h>
#include <vmmlib/lowpassFilter.hpp>

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
CVTracker::CVTracker( eq::Observer* observer, const uint32_t camera )
    : observer_( observer )
    , camera_( camera )
    , capture_( cvCaptureFromCAM( camera_ ))
    , running_( false )
{
	if( !capture_ )
    {
        LBWARN << "Did not find OpenCV camera " << camera_ << std::endl;
        return;
    }

    if( !faceDetector_.load( FACE_CONFIG ))
    {
        std::string config = FACE_CONFIG;
        config.replace( config.find( "OpenCV" ), 6, "opencv" );
        if( !faceDetector_.load( config ))
        {
            LBWARN << "Cannot set up face detector using " << FACE_CONFIG
                   << " or " << config << std::endl;

            cvReleaseCapture( &capture_ );
            capture_ = 0;
            return;
        }
    }

    if( !eyeDetector_.load( EYE_CONFIG ))
    {
        std::string config = FACE_CONFIG;
        config.replace( config.find( "OpenCV" ), 6, "opencv" );
        if( !eyeDetector_.load( config ))
        {
            LBWARN << "Can't set up eye detector using " << EYE_CONFIG << " or "
                   << config << std::endl;

            cvReleaseCapture( &capture_ );
            capture_ = 0;
            return;
        }
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
    eq::Config* config = observer_->getConfig();
    const uint128_t originator = observer_->getID();

    vmml::LowpassFilter< 5, Vector3f > position( .3f );
    vmml::LowpassFilter< 5, float > roll( .3f );
    vmml::LowpassFilter< 5, float > headEyeRatio( .3f );
    Matrix4f head;

    headEyeRatio.add( .4f ); // initial guesses
    float width = 0.f;
    bool isEyeWidth = false;

    while( running_ )
    {
        const cv::Mat frame = cvQueryFrame( capture_ );
        if( frame.empty( ))
        {
            LBWARN << "Failure to grab a video frame, bailing" << std::endl;
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
        Vector3f center( 0.f, 0.f, 0.f );
        Matrix3f rotation;

        if( eyes.size() == 2 )
        {
            const Vector2f left( face.x + eyes[0].x + eyes[0].width * .5f,
                                 face.y + eyes[0].y + eyes[0].height * .5f );
            const Vector2f right( face.x + eyes[1].x + eyes[1].width * .5f,
                                  face.y + eyes[1].y + eyes[1].height * .5f );
            center = (left + right) * .5f;
            center.z() = (right-left).length() / float(CAPTURE_WIDTH);

            // low pass smooth filter of roll angle
            roll.add( atanf( fabs( left.y() - right.y( )) /
                             fabs( left.x() - right.x( ))));
            rotation.array[ 0 ] = cosf( *roll );
            rotation.array[ 1 ] = sinf( *roll );
            rotation.array[ 4 ] = -rotation.array[ 1 ];
            rotation.array[ 5 ] =  rotation.array[ 0 ];
            head.setSubMatrix( rotation, 0, 0 );

            if( !isEyeWidth && width > 0.f )
            {
                headEyeRatio.add( center.z() / width );
                isEyeWidth = true;
            }
            width = center.z();
        }
        else
        {
            center.x() = face.x + face.width * .5f;
            center.y() = face.y + face.height * .33f; // eyes are in upper third
            center.z() = face.width / float(CAPTURE_WIDTH);

            if( isEyeWidth && width > 0.f )
            {
                headEyeRatio.add( width / center.z() );
                isEyeWidth = false;
            }
            width = center.z();
            center.z() *= *headEyeRatio;
        }
        center.x() = 2.f * ( -center.x() / float( CAPTURE_WIDTH ) + .5f );
        center.y() = 2.f * ( -center.y() / float( CAPTURE_HEIGHT ) + .5f );

        // 20 cm macro distance, 2 m tele, inverted scale
        center.z() *= 4.f;
        if( center.z() < 0.f )
            center.z() = 0.f;
        if( center.z() > 1.f )
            center.z() = 1.f;
        center.z() = .2f + (1.f - center.z()) * 2.f;
        position.add( center ); // low pass smooth filter

        // emit
        head.setTranslation( position.get( ));

        LBVERB << "head " << *position << " roll " << *roll << " eyes "
               << (eyes.size() == 2) <<  " h->e " << *headEyeRatio << std::endl;
        config->sendEvent( Event::OBSERVER_MOTION ) << originator << head;
    }

    running_ = false;
}

}
}
