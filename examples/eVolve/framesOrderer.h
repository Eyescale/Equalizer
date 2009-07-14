
/* Copyright (c) 2007       Maxim Makhinya
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

#ifndef EVOLVE_FRAMES_ORDERER_H
#define EVOLVE_FRAMES_ORDERER_H

#include <eq/eq.h>

namespace eVolve
{
void orderFrames( eq::FrameVector&    frames,
                  const eq::Matrix4d& modelviewM,
                  const eq::Matrix3d& modelviewITM,
                  const eq::Matrix4f& rotation,
                  const bool          orthographic );
}

#endif //EVOLVE_FRAMES_ORDERER_H
