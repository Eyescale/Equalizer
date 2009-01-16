
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_LAYOUT_H
#define EQSERVER_LAYOUT_H

#include "types.h"

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace server
{
    /**
     * The layout. @sa eq::Layout
     */
    class Layout
    {
    public:
        /** 
         * Constructs a new Layout.
         */
        Layout(){}

        /** Creates a new, deep copy of a layout. */
        Layout( const Layout& from );

        /** Destruct this layout. */
        virtual ~Layout(){}

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Set the name of this layout.
         *
         * The names is used by the canvas referenc layouts in the config file.
         */
        void setName( const std::string& name ) { _name = name; }

        /** @return the name of this layout. */
        const std::string& getName() const      { return _name; }

        /** Add a new view to this layout. */
        void addView( View* view ) { _views.push_back( view ); }
        
        /** Get the list of views. */
        const ViewVector& getViews() const { return _views; }
        //*}

        /**
         * @name Operations
         */
        //*{
        //*}
        
    protected:

    private:
        /** The name of this layout. */
        std::string _name;

        /** Child views on this layout. */
        ViewVector _views;
    };

    std::ostream& operator << ( std::ostream& os, const Layout* layout);
}
}
#endif // EQSERVER_LAYOUT_H
