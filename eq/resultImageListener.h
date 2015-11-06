
/* Copyright (c) 2015, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_RESULTIMAGELISTENER_H
#define EQ_RESULTIMAGELISTENER_H

#include <eq/types.h>

namespace eq
{

/**
 * The result image listener gets notified on new images produced by
 * destination channels.
 *
 * @sa Channel::addResultImageListener
 * @version 1.9
 */
class ResultImageListener
{
public:
    /** Construct a new result image listener. @version 1.9 */
    ResultImageListener() {}

    /** Destruct the result image listener. @version 1.9 */
    virtual ~ResultImageListener() {}

    /**
     * Notify on new image, called from rendering thread in
     * Channel::frameViewFinish().
     *
     * @param channel the destination channel
     * @param image the new image, valid only in the current frame
     * @version 1.9
     */
    virtual void notifyNewImage( Channel& channel, const Image& image ) = 0;

private:
    ResultImageListener( const ResultImageListener& ) = delete;
    ResultImageListener& operator=( const ResultImageListener& ) = delete;
};

}

#endif // EQ_RESULTIMAGELISTENER_H
