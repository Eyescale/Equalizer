
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/client/frustum.h>        // base class
#include <eq/client/viewport.h>       // member
#include <eq/client/visitorResult.h>  // enum

namespace eq
{
namespace server
{
    class View;
}
    class ViewVisitor;

    /**
     * A View is a 2D area of a Layout. It is a view of the application's data
     * on a model, in the sense used by the MVC pattern. It can be a scene,
     * viewing mode, viewing position, or any other representation of the
     * application's data.
     */
    class View : public Frustum
    {
    public:
        EQ_EXPORT View();
        EQ_EXPORT virtual ~View();

        /** @name Data Access. */
        //*{
        /** @return the viewport of the view. */
        EQ_EXPORT const Viewport& getViewport() const;
        //*}

        /** @name Operations */
        //*{
        /** 
         * Traverse this view using a view visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ViewVisitor& visitor );
        //*}
        
    protected:
        /** @sa Object::serialize() */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint64_t dirtyBits );

        /** @sa Object::deserialize() */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_VIEWPORT   = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 3,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 4,
        };

    private:
        friend class Layout;
        Layout* _layout;

        friend class server::View;
        Viewport    _viewport;
        std::string _name;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const View& view );
}

#endif //EQ_VIEW_H
