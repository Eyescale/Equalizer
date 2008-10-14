
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/client/projection.h>  // member
#include <eq/client/wall.h>        // member
#include <eq/net/object.h>         // base class

namespace eq
{
    /**
     * A View describes a projection of the scene onto a coherent 2D area,
     * typically a display driven by a destination Channel.
     */
    class EQ_EXPORT View : public net::Object
    {
    public:
        /** The type of the latest specified view. */
        enum Type
        {
            TYPE_NONE,        //!< No view has been specified, invalid view
            TYPE_WALL,        //!< A wall description has been set last
            TYPE_PROJECTION   //!< A projection description has been set last
        };

        View();
        View( const View& view );
        View( const Type& last, const Wall& wall, const Projection& projection);
            

        virtual ~View();

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
        } 
        _data;

        bool _dirty;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const View& view );
}

#endif //EQ_VIEW_H
