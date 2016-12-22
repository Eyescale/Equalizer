
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_LAYOUT_H
#define EQFABRIC_LAYOUT_H

#include <eq/fabric/object.h>         // base class
#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/types.h>
#include <string>

namespace eq
{
namespace fabric
{
/** Base data transport class for layouts. @sa eq::Layout */
// cppcheck-suppress noConstructor
template< class C, class L, class V > class Layout : public Object
{
public:
    /** @name Data Access */
    //@{
    /** A vector of pointers to views. @version 1.0 */
    typedef std::vector< V* > Views;
    /** The layout visitor type. @version 1.0 */
    typedef ElementVisitor< L, LeafVisitor< V > > Visitor;

    /** @return the current config. @version 1.0 */
    C* getConfig() { return _config; }

    /** @return the current config. @version 1.0 */
    const C* getConfig() const { return _config; }

    /** Get the list of views. @version 1.0 */
    const Views& getViews() const { return _views; }

    /**
     * @return true if the layout is activated in at least one canvas.
     * @version 1.1.5
     */
    EQFABRIC_INL bool isActive() const;

    /** @internal @return the view of the given path. */
    V* getView( const ViewPath& path );

    /** @internal @return the first view of the given name. */
    V* findView( const std::string& name );

    /** @internal @return the index path to this layout. */
    LayoutPath getPath() const;
    //@}

    /** @name Operations */
    //@{
    /**
     * Traverse this layout and all children using a layout visitor.
     *
     * @param visitor the visitor.
     * @return the result of the visitor traversal.
     * @version 1.0
     */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor );

    /** Const-version of accept(). @version 1.0 */
    EQFABRIC_INL VisitorResult accept( Visitor& visitor ) const;

    /**
     * Resize the underlying channels to form a layout of the given size.
     *
     * Resizes the virtual canvas so that the union of output channels covers
     * the given pixel viewport. Relative or absolute addressing of the
     * channel's viewport or pixel viewport is retained. Windows are resized as
     * necessary, so that the channel has the correct size (for channels with
     * relative viewports) or the channel fits into the window (for channels
     * with absolute pixel viewports).
     *
     * @param pvp the full layout size in pixels.
     * @version 2.1
     */
    EQFABRIC_INL void setPixelViewport( const PixelViewport& pvp );

    /** @return the last set pixel viewport. @version 2.1 */
    EQFABRIC_INL const PixelViewport& getPixelViewport() const;

    void create( V** view ); //!< @internal
    void release( V* view ); //!< @internal
    //@}

protected:
    /** @internal Construct a new layout. */
    EQFABRIC_INL explicit Layout( C* config );

    /** @internal Destruct this layout. */
    EQFABRIC_INL virtual ~Layout();

    /** @internal */
    EQFABRIC_INL virtual void attach( const uint128_t& id,
                                      const uint32_t instanceID );

    /** @internal */
    EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                         const uint64_t dirtyBits );
    /** @internal */
    EQFABRIC_INL virtual void deserialize( co::DataIStream& is,
                                           const uint64_t dirtyBits );

    EQFABRIC_INL virtual void notifyDetach(); //!< @internal
    virtual void notifyViewportChanged() {} //!< @internal

    /** @internal */
    EQFABRIC_INL virtual void setDirty( const uint64_t bits );

    /** @internal */
    enum DirtyBits
    {
        DIRTY_VIEWS      = Object::DIRTY_CUSTOM << 0,
        DIRTY_VIEWPORT   = Object::DIRTY_CUSTOM << 1,
        DIRTY_LAYOUT_BITS = DIRTY_VIEWS | DIRTY_VIEWPORT | DIRTY_OBJECT_BITS
    };

    /** @internal @return the bits to be re-committed by the master. */
    virtual uint64_t getRedistributableBits() const
    { return DIRTY_LAYOUT_BITS; }

private:
    /** The parent Config. */
    C* const _config;

    /** Child views on this layout. */
    Views _views;

    PixelViewport _pvp; //!< application-provided pixel viewport

    template< class, class, class > friend class View;
    friend class Object;
    void _addChild( V* view );
    bool _removeChild( V* view );

    /** @internal */
    EQFABRIC_INL virtual uint128_t commit( const uint32_t incarnation );

    template< class O > void _removeObserver( const O* observer );
    template< class, class, class, class, class, class,
              class > friend class Config;

    typedef co::CommandFunc< Layout< C, L, V > > CmdFunc;
    bool _cmdNewView( co::ICommand& command );
    bool _cmdNewViewReply( co::ICommand& command );
};

template< class C, class L, class V >
std::ostream& operator << ( std::ostream&, const Layout< C, L, V >& );
}
}
#endif // EQFABRIC_LAYOUT_H
