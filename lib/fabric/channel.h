
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_CHANNEL_H
#define EQFABRIC_CHANNEL_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/paths.h>
#include <eq/fabric/renderContext.h> // member
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
namespace fabric
{
    /** Base data transport class for channels. @sa eq::Channel */
    template< class W, class C > class Channel : public Object
    {
    public: 
        typedef LeafVisitor< C > Visitor; //!< The channel visitor type
        typedef W Parent; //!< The parent window type

        /** 
         * The drawable format defines the components used as an alternate
         * drawable for this cannel. If an alternate drawable is configured, the
         * channel uses the appropriate targets in place of the window's frame
         * buffer.
         * @version 1.0
         */
        enum Drawable
        {
            FB_WINDOW   = EQ_BIT_NONE, //!< Use the window's frame buffer
            FBO_COLOR   = EQ_BIT1,     //!< Use an FBO for color values
            FBO_DEPTH   = EQ_BIT2,     //!< Use an FBO for depth values
            FBO_STENCIL = EQ_BIT3      //!< Use an FBO for stencil values
        };
        
        /** @name Data Access */
        //@{
        /** @internal Initialize this channel (calls virtual methods) */
        void init();

        /** @return the parent window. @version 1.0 */
        W* getWindow() { return _window; }

        /** @return the parent window. @version 1.0 */
        const W* getWindow() const { return _window; }

        /** Update the native view identifier and version. @internal */
        void setViewVersion( const net::ObjectVersion& view );

        /** Set the channel's pixel viewport wrt its parent window. @internal */
        void setPixelViewport( const PixelViewport& pvp );

        /** Set the channel's viewport wrt its parent window. @internal */
        EQFABRIC_EXPORT void setViewport( const Viewport& vp );

        /** Notification that the vp/pvp has changed. @internal */
        virtual void notifyViewportChanged();

        /** @return the native pixel viewport. @version 1.0 */
        const PixelViewport& getNativePixelViewport() const
            { return _data.nativeContext.pvp; }

        /** @return true if a viewport was specified last. @version 1.0 */
        bool hasFixedViewport() const { return _data.fixedVP; }

        /** 
         * Set the near and far planes for this channel.
         * 
         * The given near and far planes update the current perspective and
         * orthographics frustum accordingly. Furthermore, they will be used in
         * the future by the server to compute the frusta.
         *
         * @param nearPlane the near plane.
         * @param farPlane the far plane.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setNearFar( const float nearPlane, 
                                         const float farPlane);

        /** @return a fixed unique color for this channel. @version 1.0 */
        const Vector3ub& getUniqueColor() const { return _color; }

        /**
         * @return the channel's framebuffer attachment configuration.
         * @version 1.0
         */
        uint32_t getDrawable() const { return _drawable; }

        /** 
         * Traverse this channel using a channel visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor ) const;

        /** @warning Undocumented - may not be supported in the future */
        EQFABRIC_EXPORT void setMaxSize( const Vector2i& size );

        void setOverdraw( const Vector4i& overdraw ); //!< @internal
        const Vector2i& getMaxSize()  const { return _maxSize; } //!< @internal

        /** @return the index path to this channel. @internal */
        EQFABRIC_EXPORT ChannelPath getPath() const;

        EQFABRIC_EXPORT virtual void backup(); //!< @internal
        EQFABRIC_EXPORT virtual void restore(); //!< @internal
        //@}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task. If they are called outside of a frame task method, they
         * return the channel's native parameter, e.g., a placeholder value for
         * the task decomposition parameters.
         */
        //@{
        /** @return the current draw buffer for glDrawBuffer. @version 1.0 */
        uint32_t getDrawBuffer() const { return _context->buffer; }

        /** @return the current read buffer for glReadBuffer. @version 1.0 */
        uint32_t getReadBuffer() const { return _context->buffer; }

        /** @return the current color mask for glColorMask. @version 1.0 */
        const ColorMask& getDrawBufferMask() const
            { return _context->bufferMask; }

        /**
         * @return the current pixel viewport for glViewport and glScissor.
         * @version 1.0
         */
        const PixelViewport& getPixelViewport() const { return _context->pvp; }

        /**
         * @return the current perspective frustum for glFrustum.
         * @version 1.0
         */
        const Frustumf& getFrustum() const { return _context->frustum; }

        /**
         * @return the current orthographic frustum for glOrtho.
         * @version 1.0
         */
        const Frustumf& getOrtho() const { return _context->ortho; }

        /**
         * Return the view matrix.
         *
         * The view matrix is part of the GL_MODEL*VIEW* matrix, and is
         * typically applied first to the GL_MODELVIEW matrix.
         * 
         * @return the head transformation matrix
         * @version 1.0
         */
        const Matrix4f& getHeadTransform() const
            { return _context->headTransform; }

        /**
         * @return the fractional viewport wrt the destination view.
         * @version 1.0
         */
        const Viewport& getViewport() const { return _context->vp; }

        /**
         * @return the database range for the current rendering task.
         * @version 1.0
         */
        const Range& getRange() const { return _context->range; }

        /**
         * @return the pixel decomposition for the current rendering task.
         * @version 1.0
         */
        const Pixel& getPixel() const { return _context->pixel; }

        /**
         * @return the subpixel decomposition for the current rendering task.
         * @version 1.0
         */
        const SubPixel& getSubPixel() const { return _context->subpixel; }

        /**
         * @return the up/downscale zoom factor for the current rendering task.
         * @version 1.0
         */
        const Zoom& getZoom() const { return _context->zoom; }

        /**
         * @return the DPlex period for the current rendering task.
         * @version 1.0
         */
        uint32_t getPeriod() const { return _context->period; }

        /**
         * @return the DPlex phase for the current rendering task.
         * @version 1.0
         */
        uint32_t getPhase() const { return _context->phase; }

        /**
         * Get the channel's current position wrt the destination channel.
         *
         * Note that computing this value from the current viewport and pixel
         * viewport is inaccurate because it neglects rounding errors of the
         * pixel viewport done by the server.
         *
         * @return the channel's current position wrt the destination channel.
         * @version 1.0
         */
        const Vector2i& getPixelOffset() const { return _context->offset; }

        /** @return the currently rendered eye pass. @version 1.0 */
        Eye getEye() const { return _context->eye; }

        /** @warning Undocumented - may not be supported in the future */
        const Vector4i& getOverdraw() const { return _context->overdraw; }

        /** @warning Undocumented - may not be supported in the future */
        uint32_t getTaskID() const { return _context->taskID; }
        //@}

        /** @name Attributes */
        //@{
        // Note: also update string array initialization in channel.cpp
        /** Integer attributes for a channel. @version 1.0 */
        enum IAttribute
        {
            /** Statistics gathering mode (OFF, FASTEST [ON], NICEST) */
            IATTR_HINT_STATISTICS,
            /** Use a send token for output frames (OFF, ON) */
            IATTR_HINT_SENDTOKEN,
            IATTR_LAST,
            IATTR_ALL = IATTR_LAST + 5
        };
        
        /** @return the value of an integer attribute. @version 1.0 */
        EQFABRIC_EXPORT int32_t getIAttribute( const IAttribute attr ) const;
        /** @return the name of an integer attribute. @version 1.0 */
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}
        
    protected:
        /** Construct a new channel. @internal */
        EQFABRIC_EXPORT Channel( W* parent );

        /** Construct a copy of a channel (view/segment dest). @internal */
        Channel( const Channel& from );

        /** Destruct the channel. @internal */
        EQFABRIC_EXPORT virtual ~Channel();

        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_EXPORT virtual void setDirty( const uint64_t bits );

        void setDrawable( const uint32_t drawable ); //!< @internal

        /** @name Render context access */
        //@{
        /** @internal Override the channel's native render context. */
        void overrideContext( RenderContext& context ) { _context = &context; }

        /** @internal Re-set the channel's native render context. */
        void resetContext() { _context = &_data.nativeContext; }

        /** @internal @return the current render context. */
        const RenderContext& getContext() const { return *_context; }

        /** @internal @return the native render context. */
        const RenderContext& getNativeContext() const
            { return _data.nativeContext; }
        //@}

        /** @internal */
        void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[attr] = value; setDirty( DIRTY_ATTRIBUTES ); }

        /** @internal */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        enum DirtyBits
        {
            DIRTY_ATTRIBUTES = Object::DIRTY_CUSTOM << 0, // 64
            DIRTY_VIEWPORT   = Object::DIRTY_CUSTOM << 1, // 128
            DIRTY_MEMBER     = Object::DIRTY_CUSTOM << 2, // 256
            DIRTY_FRUSTUM    = Object::DIRTY_CUSTOM << 3, // 512
        };

        /** The parent window. */
        W* const _window;

        struct BackupData
        {
            BackupData() : fixedVP( true ) {}

            /** The native render context parameters of this channel. */
            RenderContext nativeContext;

            /** true if the vp is immutable, false if the pvp is immutable */
            bool fixedVP;
        }
            _data, _backup;

        /** The current rendering context. */
        RenderContext* _context;

        /** A unique color assigned by the server during config init. */
        Vector3ub _color;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];

        /** An alternate drawable config. */
        uint32_t _drawable;

        /** Overdraw limiter */
        Vector2i    _maxSize;
        
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQFABRIC_CHANNEL_H

