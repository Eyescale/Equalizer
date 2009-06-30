
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EVOLVE_CHANNEL_H
#define EVOLVE_CHANNEL_H

#include "eVolve.h"
#include "frameData.h"

#include <eq/eq.h>
#include <eq/client/types.h>

namespace eVolve
{
    class InitData;

    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent );

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );

        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );
        virtual void frameViewFinish( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

        void clearViewport( const eq::PixelViewport &pvp );

        void frameClear( const uint32_t frameID );

    private:
        void _startAssemble();

        void _orderFrames( eq::FrameVector& frames );

        void _calcMVandITMV( eq::Matrix4f& modelviewM, 
                             eq::Matrix3f& modelviewITM ) const;

        const FrameData::Data& _getFrameData() const;

        eq::Vector3f _bgColor; //!< background color
        eq::Frame _frame;     //!< Readback buffer for DB compositing
        eq::Range _drawRange; //!< The range from the last draw of this frame
    };

}

#endif // EVOLVE_CHANNEL_H

