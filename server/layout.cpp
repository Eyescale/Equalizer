
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "layout.h"

#include "view.h"

namespace eq
{
namespace server
{

Layout::Layout( const Layout& from )
        : _name( from._name )
{
    for( ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        _views.push_back( new View( **i ));
    }
}

}
}
