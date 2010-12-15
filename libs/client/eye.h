
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_EYE_H
#define EQ_EYE_H

#include <eq/fabric/eye.h> // 'base' class

namespace eq
{
    typedef fabric::Eye Eye;

    /** @cond IGNORE */
    using fabric::EYE_UNDEFINED;
    using fabric::EYE_CYCLOP;
    using fabric::EYE_LEFT;
    using fabric::EYE_RIGHT;
    using fabric::EYES_STEREO;
    using fabric::EYES_ALL;
    using fabric::NUM_EYES;
    /** @endcond */
}

#endif // EQ_EYE_H
