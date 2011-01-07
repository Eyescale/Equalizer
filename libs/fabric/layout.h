
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
namespace fabric
{
    /** Base data transport class for layouts. @sa eq::Layout */
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
        EQFABRIC_INL VisitorResult accept( Visitor& visitor )
            const;

        void create( V** view ); //!< @internal
        void release( V* view ); //!< @internal
        //@}
        
    protected:
        /** @internal Construct a new layout. */
        EQFABRIC_INL Layout( C* config );

        /** @internal Destruct this layout. */
        EQFABRIC_INL virtual ~Layout();

        /** @internal */
        EQFABRIC_INL virtual void attach( const co::base::UUID& id,
                                          const uint32_t instanceID );

        /** @internal */
        EQFABRIC_INL virtual void serialize( co::DataOStream& os, 
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_INL virtual void deserialize( co::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        EQFABRIC_INL virtual void notifyDetach(); //!< @internal

        /** @internal */
        EQFABRIC_INL virtual void setDirty( const uint64_t bits );

        /** @internal */
        enum DirtyBits
        {
            DIRTY_VIEWS      = Object::DIRTY_CUSTOM << 0,
            DIRTY_LAYOUT_BITS = DIRTY_VIEWS | DIRTY_OBJECT_BITS
        };

        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_LAYOUT_BITS; }

    private:
        /** The parent Config. */
        C* const _config;

        /** Child views on this layout. */
        Views _views;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        template< class, class, class > friend class View;
        friend class Object;
        void _addChild( V* view );
        bool _removeChild( V* view );

        EQFABRIC_INL virtual uint32_t commitNB(); //!< @internal

        template< class O > void _removeObserver( const O* observer );
        template< class, class, class, class, class, class,
                  class > friend class Config;

        typedef co::CommandFunc< Layout< C, L, V > > CmdFunc;
        bool _cmdNewView( co::Command& command );
        bool _cmdNewViewReply( co::Command& command );
    };

    template< class C, class L, class V >
    std::ostream& operator << ( std::ostream&, const Layout< C, L, V >& );
}
}
#endif // EQFABRIC_LAYOUT_H
