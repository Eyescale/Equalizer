
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h>  // enum

#include <string>

namespace eq
{
    class Layout;
    namespace server { class Layout; }

namespace fabric
{
    template< typename T, typename C  > class ElementVisitor;
    template< class T > class LeafVisitor;
    template< class, class, class > class View;
    struct LayoutPath;
    struct ViewPath;

    /**
     * A layout groups one or more View which logically belong together.
     *
     * A layout belongs to one or more Canvas. The layout assignment can be
     * changed at run-time by the application, out of a pre-defined set of
     * layouts for each Canvas.
     * 
     * The intersection between views and segments defines which output
     * (sub-)channels are available. Neither the views nor the segments have to
     * cover the full layout or canvas, respectively. These channels are
     * typically used as a destination Channel in a compound. They are
     * automatically created when the configuration is loaded.
     */ 
    template< class C, class L, class V > class Layout : public Object
    {
    public:
        /** @name Data Access */
        //@{
        typedef ElementVisitor< L, LeafVisitor< V > > Visitor;
        typedef std::vector< V* > Views;

        /** @return the current config. */
        C* getConfig() { return _config; }

        /** @return the current Config. */
        const C* getConfig() const { return _config; }

        /** Get the list of views. */
        const Views& getViews() const { return _views; }

        /** @return the view of the given path. @internal */
        V* getView( const ViewPath& path );

        /** @return the first view of the given name. @internal */
        V* findView( const std::string& name );

        /** @return the index path to this layout. @internal */
        LayoutPath getPath() const;
        //@}

        /** @name Operations */
        //@{
        /** 
         * Traverse this layout and all children using a layout visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor )
            const;

        void create( V** view ); //!< @internal
        void release( V* view ); //!< @internal
        //@}
        
    protected:
        /** Construct a new layout. */
        EQFABRIC_EXPORT Layout( C* config );

        /** Destruct this layout. */
        EQFABRIC_EXPORT virtual ~Layout();

        EQFABRIC_EXPORT virtual void attachToSession( const uint32_t id,
                                      const uint32_t instanceID,
                                      net::Session* session ); //!< @internal

        /** @sa Object::serialize */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os, 
                                                const uint64_t dirtyBits );
        /** @sa Object::deserialize */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        EQFABRIC_EXPORT virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_EXPORT virtual void setDirty( const uint64_t bits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        enum DirtyBits
        {
            DIRTY_VIEWS      = Object::DIRTY_CUSTOM << 0,
        };

        /** The parent Config. */
        C* const _config;

        /** Child views on this layout. */
        Views _views;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class, class > friend class View;
        void _addView( V* view );
        bool _removeView( V* view );

        EQFABRIC_EXPORT virtual uint32_t commitNB(); //!< @internal

        typedef net::CommandFunc< Layout< C, L, V > > CmdFunc;
        bool _cmdNewView( net::Command& command );
        bool _cmdNewViewReply( net::Command& command );
    };

    template< class C, class L, class V >
    std::ostream& operator << ( std::ostream&, const Layout< C, L, V >& );
}
}
#endif // EQFABRIC_LAYOUT_H
