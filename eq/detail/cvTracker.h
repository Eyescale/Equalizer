
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

#ifndef EQ_CVTRACKER_H
#define EQ_CVTRACKER_H

#include <eq/types.h>
#include <opencv2/opencv.hpp>

namespace eq
{
namespace detail
{
/**
 * @internal
 * An OpenCV head tracker running in a separate thread for performance.
 */
class CVTracker : public lunchbox::Thread
{
public:
    /** Construct a new tracker. */
    CVTracker( eq::Observer* observer, const uint32_t camera );

    /** Destruct this tracker. */
    virtual ~CVTracker();

    /** @return true of the tracker is working. */
    bool isGood() const { return capture_; }

protected:
    virtual void run();

private:
    eq::Observer* observer_;
    const uint32_t camera_;
    CvCapture* capture_;
    cv::CascadeClassifier faceDetector_;
    cv::CascadeClassifier eyeDetector_;

    bool running_;
};
}
}
#endif // EQ_CVTRACKER_H
