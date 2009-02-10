
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
    class Config;
    class Layout;

    /** 
     * Extends the eq::View to implement server-side logic.
     */
    class View : public eq::View
    {
    public:
        View();

        /** Creates a new, deep copy of a view. */
        View( const View& from );

        virtual ~View();

        /** @name Operations */
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
        
        /** @name Data Access. */
        //*{
        /** Set the area covered by this wrt it parent Layout. */
        void setViewport( const Viewport& viewport );

        /** @return the config of this view. */
        Config* getConfig();

        /** @return the config of this view. */
        const Config* getConfig() const;

        /** 
         * Adds a new destination channel to this view.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel );
        
        /** 
         * Removes a destination channel from this view.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeChannel( Channel* channel );
        
        /** @return the vector of channels. */
        const ChannelVector& getChannels() const{ return _channels; }
        
    protected:

    private:
        /** The parent Layout. */
        Layout* _layout;
        friend class Layout;

        /** The list of channels. */
        ChannelVector _channels;
    };

    std::ostream& operator << ( std::ostream& os, const View* view );
}
}
#endif // EQ_VIEW_H
