
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CHANNELUPDATEVISITOR_H
#define EQSERVER_CHANNELUPDATEVISITOR_H

#include "compoundVisitor.h" // base class
#include "types.h"

#include <eq/fabric/eye.h>         // member

namespace eq
{
namespace fabric
{
    class ColorMask;
    class RenderContext;
}

namespace server
{
    class Channel;
    class FrustumData;
    
    /** The compound visitor generating the draw tasks for a channel. */
    class ChannelUpdateVisitor : public CompoundVisitor
    {
    public:
        ChannelUpdateVisitor( Channel* channel, const uint128_t frameID, 
                              const uint32_t frameNumber );
        virtual ~ChannelUpdateVisitor() {}

        void setEye( const fabric::Eye eye ) { _eye = eye; }

        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( const Compound* compound );
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( const Compound* compound );
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( const Compound* compound );

        bool isUpdated() const { return _updated; }

    private:
        Channel*        _channel;
        fabric::Eye     _eye;
        const uint128_t _frameID;
        const uint32_t  _frameNumber;
        bool            _updated;

        bool _skipCompound( const Compound* compound );

        void _updateDrawFinish( const Compound* compound ) const;
        void _updateFrameRate( const Compound* compound ) const;

        uint32_t _getDrawBuffer( const Compound* compound ) const;
        fabric::ColorMask _getDrawBufferMask( const Compound* compound ) const;

        void _setupRenderContext( const Compound* compound,
                                  fabric::RenderContext& context );

        void _computeFrustum( const Compound* compound,
                              fabric::RenderContext& context );
        Vector3f _getEyePosition( const Compound* compound ) const;
        const Matrix4f& _getInverseHeadMatrix( const Compound* compound )
            const;

        void   _computeFrustumCorners( Frustumf& frustum,
                                       const Compound* compound,
                                       const FrustumData& frustumData,
                                       const Vector3f& eye,
                                       const bool ortho );
        void _updatePostDraw( const Compound* compound, 
                              const fabric::RenderContext& context );
        void _updateAssemble( const Compound* compound,
                              const fabric::RenderContext& context );
        void _updateReadback( const Compound* compound,
                              const fabric::RenderContext& context );  
        void _updateViewStart( const Compound* compound,
                               const fabric::RenderContext& context );
        void _updateViewFinish( const Compound* compound,
                                const fabric::RenderContext& context );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
