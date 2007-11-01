
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNELUPDATEVISITOR_H
#define EQS_CHANNELUPDATEVISITOR_H

#include "constCompoundVisitor.h" // base class

#include <eq/client/colorMask.h>
#include <eq/client/eye.h>
#include <eq/client/renderContext.h>
#include <eq/client/windowSystem.h>

namespace eqs
{
    class Channel;
    
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
        virtual Compound::VisitorResult visitPre( const Compound* compound );
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( const Compound* compound );
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( const Compound* compound );

    private:
        Channel*       _channel;
        eq::Eye        _eye;
        const uint32_t _frameID;
        const uint32_t _frameNumber;

        void _updateDrawFinish( const Compound* compound ) const;
        GLenum _getDrawBuffer() const;
        eq::ColorMask _getDrawBufferMask( const Compound* compound ) const;

        void _setupRenderContext( const Compound* compound,
                                  eq::RenderContext& context );
        void _computeFrustum( const Compound* compound,
                              eq::RenderContext& context );
        void _updatePostDraw( const Compound* compound, 
                              const eq::RenderContext& context );
        void _updateAssemble( const Compound* compound,
                              const eq::RenderContext& context );
        void _updateReadback( const Compound* compound,
                              const eq::RenderContext& context );  
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
