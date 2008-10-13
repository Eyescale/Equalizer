
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef CLIENT_VIEW_H
#define CLIENT_VIEW_H

#include <eq/net/object.h>  // base class

namespace eq
{
    class View : public net::Object
    {
    public:
        View();
        View( const View& view );
        virtual ~View(){}

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );
    };
}

#endif //CLIENT_VIEW_H
