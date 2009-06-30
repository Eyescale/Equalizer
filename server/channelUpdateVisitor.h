
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "constCompoundVisitor.h" // base class

#include <eq/client/colorMask.h>
#include <eq/client/eye.h>
#include <eq/client/renderContext.h>
#include <eq/client/windowSystem.h>

namespace eq
{
namespace server
{
    class Channel;
    class FrustumData;
    
    /**
     * The compound visitor generating the draw tasks for a channel.
     */
    class ChannelUpdateVisitor : public ConstCompoundVisitor
    {
    public:
        ChannelUpdateVisitor( Channel* channel, const uint32_t frameID, 
                              const uint32_t frameNumber );
        virtual ~ChannelUpdateVisitor() {}

        void setEye( const eq::Eye eye ) { _eye = eye; }

        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( const Compound* compound );
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( const Compound* compound );
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( const Compound* compound );

        bool isUpdated() const { return _updated; }

    private:
        Channel*       _channel;
        eq::Eye        _eye;
        const uint32_t _frameID;
        const uint32_t _frameNumber;
        bool           _updated;

        bool _skipCompound( const Compound* compound );

        void _updateDrawFinish( const Compound* compound ) const;
        void _updateFrameRate( const Compound* compound ) const;

        GLenum _getDrawBuffer() const;
        eq::ColorMask _getDrawBufferMask( const Compound* compound ) const;

        void _setupRenderContext( const Compound* compound,
                                  eq::RenderContext& context );

        void _computeFrustum( const Compound* compound,
                              eq::RenderContext& context );
        eq::Vector3f _getEyePosition( const Compound* compound ) const;
        const eq::Matrix4f& _getInverseHeadMatrix( const Compound* compound )
            const;

        void   _computeFrustumCorners( eq::Frustumf& frustum,
                                       const Compound* compound,
                                       const FrustumData& frustumData,
                                       const eq::Vector3f& eye,
                                       const bool ortho );
        void _updatePostDraw( const Compound* compound, 
                              const eq::RenderContext& context );
        void _updateAssemble( const Compound* compound,
                              const eq::RenderContext& context );
        void _updateReadback( const Compound* compound,
                              const eq::RenderContext& context );  
        void _updateViewStart( const Compound* compound,
                               const eq::RenderContext& context );
        void _updateViewFinish( const Compound* compound,
                                const eq::RenderContext& context );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
