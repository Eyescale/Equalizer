
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

        /** Set the eye separation. */
        void setEyeBase( const float eyeBase );

        /** Update the eye positions based on the head matrix. */
        void updateHead();

        // Used by Config
        virtual void getInstanceData( net::DataOStream& os )
            { eq::View::getInstanceData( os ); }

    protected:
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        ViewData& _data;

        /** Update the view (wall/projection. */
        void _updateView();
    };
}
}
#endif // EQ_VIEW_H
