
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

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

//#include "frameData.h"
#include "typedefs.h"

#include <eq/eq.h>


namespace eqPly
{
    class FrameData;
    class InitData;

    /**
     * The rendering entity, updating a part of a Window.
     */
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent ) : eq::Channel( parent ) {}

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );
        virtual void frameViewFinish( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

    private:
        void _drawModel( const Model* model );
        void _drawOverlay();
        void _drawHelp();
        void _updateNearFar( const mesh::BoundingSphere& boundingSphere );
        void _initFrustum( eq::FrustumCullerf& frustum, 
                           const mesh::BoundingSphere& boundingSphere );

        const FrameData& _getFrameData() const;
        const Model*     _getModel();
    };
}



#endif // EQ_PLY_CHANNEL_H

