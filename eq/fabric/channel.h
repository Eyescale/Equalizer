
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
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

    /** @name Data Access */
    //@{
    /** @internal Initialize this channel (calls virtual methods) */
    void init();

    /** @return the parent window. @version 1.0 */
    W* getWindow() { return _window; }

    /** @return the parent window. @version 1.0 */
    const W* getWindow() const { return _window; }

    /** @internal @return the native view identifier and version. */
    bool isDestination() const
        { return _data.nativeContext.view.identifier != 0;}

    /** @internal Update the native view identifier and version. */
    void setViewVersion( const co::ObjectVersion& view );

    /** @internal @return the native view identifier and version. */
    const co::ObjectVersion& getViewVersion() const
        { return _data.nativeContext.view; }

    /** @internal Set the channel's pixel viewport wrt its parent window. */
    void setPixelViewport( const PixelViewport& pvp );

    /** @internal Set the channel's viewport wrt its parent window. */
    EQFABRIC_INL void setViewport( const Viewport& vp );

    /** @internal Notification that the vp/pvp has changed. */
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
     * orthographics frustum accordingly. Furthermore, they will be used in the
     * future by the server to compute the frusta.
     *
     * @param nearPlane the near plane.
     * @param farPlane the far plane.
     * @version 1.0
     */
    EQFABRIC_INL void setNearFar( const float nearPlane, const float farPlane );

    /**
     * Perform a depth-first traversal of this channel.
     *
     * @param visitor the visitor.
     * @return the result of the visitor traversal.
     * @version 1.0
     */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor );

    /** Const-version of accept(). @version 1.0 */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor ) const;

    /**
     * Set the capabilities supported by the channel
     *
     * Channel which do not support all capabilities required by the current
     * destination view do not execute any tasks. The capabilities are an
     * application-defined bit mask. By default all bits are set.
     * @version 1.0
     */
    EQFABRIC_INL void setCapabilities( const uint64_t bitmask );

    /** @return the supported capabilities. @version 1.0 */
    EQFABRIC_INL uint64_t getCapabilities() const;

    /** @warning Undocumented - may not be supported in the future */
    EQFABRIC_INL void setMaxSize( const Vector2i& size );

    void setOverdraw( const Vector4i& overdraw ); //!< @internal
    const Vector2i& getMaxSize()  const { return _maxSize; } //!< @internal

    /** @internal @return the index path to this channel. */
    EQFABRIC_INL ChannelPath getPath() const;

    EQFABRIC_INL virtual void backup(); //!< @internal
    EQFABRIC_INL virtual void restore(); //!< @internal
    //@}

    /**
     * @name Context-specific data access.
     *
     * The data returned by these methods depends on the context (callback) they
     * are called from, typically the data for the current rendering task. If
     * they are called outside of a frame task method, they return the channel's
     * native parameter, e.g., a placeholder value for the task decomposition
     * parameters.
     */
    //@{
    /** @return the current draw buffer for glDrawBuffer. @version 1.0 */
    uint32_t getDrawBuffer() const { return _context->buffer; }

    /** @return the current read buffer for glReadBuffer. @version 1.0 */
    uint32_t getReadBuffer() const { return _context->buffer; }

    /** @return the current color mask for glColorMask. @version 1.0 */
    const ColorMask& getDrawBufferMask() const { return _context->bufferMask; }

    /**
     * @return the current pixel viewport for glViewport and glScissor.
     * @version 1.0
     */
    const PixelViewport& getPixelViewport() const { return _context->pvp; }

    /**
     * Select perspective or orthographic rendering.
     *
     * Influences the behaviour of getFrustum, getHeadTransform and the
     * corresponding apply methods in eq::Channel. Intended to be overwritten by
     * the implementation to select orthographic rendering.
     * @version 1.0
     */
    virtual bool useOrtho() const { return false; }

    /**
     * @return the current frustum for glFrustum or glOrtho.
     * @version 1.0
     */
    const Frustumf& getFrustum() const
        { return useOrtho() ? getOrtho() : getPerspective(); }

    /**
     * @return the current perspective frustum for glFrustum.
     * @version 1.0
     */
    const Frustumf& getPerspective() const { return _context->frustum; }

    /**
     * @return the current orthographic frustum for glOrtho.
     * @version 1.0
     */
    const Frustumf& getOrtho() const { return _context->ortho; }

    /**
     * Return the view matrix.
     *
     * The view matrix is part of the GL_MODEL*VIEW* matrix, and is typically
     * applied first to the GL_MODELVIEW matrix.
     *
     * @return the head transformation matrix
     * @version 1.0
     */
    const Matrix4f& getHeadTransform() const
        { return useOrtho() ? getOrthoTransform() : getPerspectiveTransform(); }

    /**
     * Return the perspective view matrix.
     *
     * The view matrix is part of the GL_MODEL*VIEW* matrix, and is typically
     * applied first to the GL_MODELVIEW matrix.
     *
     * @return the head transformation matrix
     * @version 1.0
     */
    const Matrix4f& getPerspectiveTransform() const
        { return _context->headTransform; }

    /**
     * Return the orthographic view matrix.
     *
     * The view matrix is part of the GL_MODEL*VIEW* matrix, and is typically
     * applied first to the GL_MODELVIEW matrix.
     *
     * @return the head transformation matrix
     * @version 1.0
     */
    const Matrix4f& getOrthoTransform() const
        { return _context->orthoTransform; }

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
    const SubPixel& getSubPixel() const { return _context->subPixel; }

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
     * viewport is inaccurate because it neglects rounding errors of the pixel
     * viewport done by the server.
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

    /** @return the current render context. */
    const RenderContext& getContext() const { return *_context; }
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

    /** String attributes. */
    enum SAttribute
    {
        SATTR_DUMP_IMAGE,
        SATTR_LAST,
        SATTR_ALL = SATTR_LAST + 5
    };

    /** @return the value of an integer attribute. @version 1.0 */
    EQFABRIC_INL int32_t getIAttribute( const IAttribute attr ) const;

    /** @return the value of a string attribute. @version 1.7.2 */
    EQFABRIC_INL
    const std::string& getSAttribute( const SAttribute attr ) const;

    /** @return the name of an integer attribute. @version 1.7.2 */
    EQFABRIC_INL static const std::string& getIAttributeString(
        const IAttribute attr );

    /** @return the name of an string attribute. @version 1.7.2 */
    EQFABRIC_INL static const std::string& getSAttributeString(
        const SAttribute attr );
    //@}

    virtual bool omitOutput() const { return false; } //!< @internal
    virtual void output( std::ostream& ) const {} //!< @internal

protected:
    /** Construct a new channel. @internal */
    EQFABRIC_INL explicit Channel( W* parent );

    /** Construct a copy of a channel (view/segment dest). @internal */
    Channel( const Channel& from );

    /** Destruct the channel. @internal */
    EQFABRIC_INL virtual ~Channel();

    /** @internal */
    EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                         const uint64_t dirtyBits );
    /** @internal */
    EQFABRIC_INL virtual void deserialize( co::DataIStream& is,
                                           const uint64_t dirtyBits );

    /** @internal @sa Serializable::setDirty() */
    EQFABRIC_INL virtual void setDirty( const uint64_t bits );

    /** @name Render context access */
    //@{
    /** @internal Override the channel's native render context. */
    void overrideContext( const RenderContext& context )
        { _overrideContext = context; _context = &_overrideContext; }

    /** @internal Re-set the channel's native render context. */
    void resetContext() { _context = &_data.nativeContext; }

    /** @internal @return the native render context. */
    const RenderContext& getNativeContext() const
        { return _data.nativeContext; }
    //@}

    /** @internal */
    void setIAttribute( const IAttribute attr, const int32_t value )
        { _iAttributes[attr] = value; setDirty( DIRTY_ATTRIBUTES ); }

    /** @internal */
    void setSAttribute( const SAttribute attr, const std::string& value )
        { _sAttributes[attr] = value; setDirty( DIRTY_ATTRIBUTES ); }

    /** @internal */
    virtual ChangeType getChangeType() const { return UNBUFFERED; }

    enum DirtyBits
    {
        DIRTY_ATTRIBUTES    = Object::DIRTY_CUSTOM << 0,
        DIRTY_VIEWPORT      = Object::DIRTY_CUSTOM << 1,
        DIRTY_MEMBER        = Object::DIRTY_CUSTOM << 2,
        DIRTY_FRUSTUM       = Object::DIRTY_CUSTOM << 3,
        DIRTY_CAPABILITIES  = Object::DIRTY_CUSTOM << 4,
        DIRTY_CHANNEL_BITS =
        DIRTY_ATTRIBUTES | DIRTY_VIEWPORT | DIRTY_MEMBER |
        DIRTY_FRUSTUM | DIRTY_OBJECT_BITS
    };

    /** @internal @return the bits to be re-committed by the master. */
    virtual uint64_t getRedistributableBits() const
        { return DIRTY_CHANNEL_BITS; }

    virtual void updateCapabilities() {} //!< @internal

private:
    /** The parent window. */
    W* const _window;

    struct BackupData
    {
        BackupData() : capabilities( LB_BIT_ALL_64 ), fixedVP( true ) {}

        /** The native render context parameters of this channel. */
        RenderContext nativeContext;

        /** Bitmask of supported capabilities */
        uint64_t capabilities;

        /** true if the vp is immutable, false if the pvp is immutable */
        bool fixedVP;
    }
        _data, _backup;

    /** Overridden context data. */
    RenderContext _overrideContext;

    /** The current rendering context, points to native or override context. */
    RenderContext* _context;

    /** Integer attributes. */
    int32_t _iAttributes[IATTR_ALL];

    /** String attributes. */
    std::string _sAttributes[SATTR_ALL];

    /** Overdraw limiter */
    Vector2i    _maxSize;
};

template< class W, class C > EQFABRIC_INL
std::ostream& operator << ( std::ostream&, const Channel< W, C >& );
}
}

#endif // EQFABRIC_CHANNEL_H
