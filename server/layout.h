
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_LAYOUT_H
#define EQSERVER_LAYOUT_H

#include "visitorResult.h" // enum
#include "types.h"

#include <eq/client/layout.h> // base class
#include <string>

namespace eq
{
namespace server
{
    class ViewPath;
    class LayoutPath;
    class LayoutVisitor;
    class ConstLayoutVisitor;

    /**
     * The layout. @sa eq::Layout
     */
    class Layout : public eq::Layout
    {
    public:
        /** 
         * Constructs a new Layout.
         */
        Layout();

        /** Creates a new, deep copy of a layout. */
        Layout( const Layout& from, Config* config );

        /** Destruct this layout. */
        virtual ~Layout();

        /**
         * @name Data Access
         */
        //*{
        /** Add a new view to this layout. */
        void addView( View* view );
        
        /** remove the view from this layout. */
        bool removeView( View* view );
        
        /** Get the list of views. */
        const ViewVector& getViews() const { return _views; }

        /** 
         * Find the first view of a given name.
         * 
         * @param name the name of the view to find
         * @return the first view with the name, or <code>0</code> if no
         *         view with the name exists.
         */
        View* findView( const std::string& name );

        Config* getConfig() { return _config; }
        const Config* getConfig() const { return _config; }

        /** @return the view of the given path. */
        View* getView( const ViewPath& path );

        /** @return the index path to this layout. */
        LayoutPath getPath() const;
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Traverse this layout and all children using a layout visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( LayoutVisitor& visitor );
        VisitorResult accept( ConstLayoutVisitor& visitor ) const;

        /** Unmap this layout and all its children. */
        void unmap();
        //*}
        
    protected:
        /** @sa Object::serialize */
        virtual void serialize( net::DataOStream& os, 
                                          const uint64_t dirtyBits );

    private:
        /** The parent Config. */
        Config* _config;
        friend class Config;

        /** Child views on this layout. */
        ViewVector _views;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

    std::ostream& operator << ( std::ostream& os, const Layout* layout);
}
}
#endif // EQSERVER_LAYOUT_H
