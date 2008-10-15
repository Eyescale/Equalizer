
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEW_H
#define EQSERVER_VIEW_H

#include <eq/client/view.h> // base class

namespace eq
{
namespace server
{
    struct ViewData;

    /** 
     * Extends the eq::View to update server-side generic view data.
     */
    class View : public eq::View
    {
    public:
        View( ViewData& data );
        View( const View& from, ViewData& data );
        virtual ~View(){}
        
        /** Set the view using a wall description. */
        void setWall( const eq::Wall& wall );
        
        /** Set the view using a projection description. */
        void setProjection( const eq::Projection& projection );

    protected:
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        ViewData& _data;

        void _updateData();
    };
}
}
#endif // EQ_VIEW_H
