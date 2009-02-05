
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEW_H
#define EQSERVER_VIEW_H

#include "viewVisitor.h"        // used in inline method
#include "types.h"

#include <eq/client/view.h>     // base class
#include <eq/client/viewport.h> // member

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

        //----- New View API
        View();
        View( const View& from );

        /** Data access. */
        //*{
        /** 
         * Set the view's area wrt its parent layout.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp );

        /** @return this view's viewport. */
        const eq::Viewport& getViewport() const { return _vp; }
        //*}

        /** Operations */
        //*{
        /** 
         * Traverse this view using a view visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ViewVisitor* visitor )
            { return visitor->visit( this ); }
        //*}

        /** 
         * Adds a new channel to this config.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel );
        
        /** 
         * Removes a node channel this config.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeChannel( Channel* channel );
        
        /** @return the vector of channels. */
        const ChannelVector& getChannels() const{ return _channels; }
        
    protected:
        virtual void applyInstanceData( net::DataIStream& is );

    private:

        /** The list of channels. */
        ChannelVector _channels;
        
        ViewData& _data;

        /** Update the view (wall/projection). */
        void _updateView();


        //----- New View API
        /** The fractional viewport with respect to the layout. */
        eq::Viewport _vp;
    };

    std::ostream& operator << ( std::ostream& os, const View* view );
}
}
#endif // EQ_VIEW_H
