
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/client/projection.h>  // member
#include <eq/client/wall.h>        // member
#include <eq/net/object.h>         // base class

namespace eq
{
    class EQ_EXPORT View : public net::Object
    {
    public:
        View();
        View( const View& view );
        virtual ~View();

        /** @name The type of the latest specified view. */
        enum Type
        {
            TYPE_NONE,
            TYPE_WALL,
            TYPE_PROJECTION
        };

        /** 
         * Set the view using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const Wall& wall );
        
        /** @return the last specified view as a wall. */
        const Wall& getWall() const { return _data.wall; }

        /** 
         * Set the view using a projection description.
         * 
         * @param projection the projection description.
         */
        void setProjection( const Projection& projection );

        /** @return the last specified view as a projection. */
        const Projection& getProjection() const { return _data.projection; }

        /** @return the type of the latest specified view. */
        Type getCurrentType() const { return _data.current; }

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }

        virtual void getInstanceData( net::DataOStream& os );
        virtual void pack( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        struct Data
        {
            Wall       wall;
            Projection projection;
            Type       current;
        } _data;

        bool _dirty;
    };
}

#endif //EQ_VIEW_H
