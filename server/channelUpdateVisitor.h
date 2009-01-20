
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
    class ViewData;
    
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

        void _updateDrawFinish( const Compound* compound ) const;
        void _updateFrameRate( const Compound* compound ) const;

        GLenum _getDrawBuffer() const;
        eq::ColorMask _getDrawBufferMask( const Compound* compound ) const;

        void _setupRenderContext( const Compound* compound,
                                  eq::RenderContext& context );

        void _computeFrustum( const Compound* compound,
                              eq::RenderContext& context );
        void   _computeFrustumCorners( vmml::Frustumf& frustum,
                                       const Compound* compound,
                                       const ViewData& viewData,
                                       const vmml::Vector3f& eye,
                                       const bool ortho );
        void _updatePostDraw( const Compound* compound, 
                              const eq::RenderContext& context );
        void _updateAssemble( const Compound* compound,
                              const eq::RenderContext& context );
        void _updateReadback( const Compound* compound,
                              const eq::RenderContext& context );  
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
