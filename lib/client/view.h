
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/client/projection.h>  // member
#include <eq/client/wall.h>        // member
#include <eq/net/object.h>         // base class

namespace eq
{
    /**
     * A View describes the projection of the scene onto a coherent 2D area,
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
        View( net::DataIStream& is );

        virtual ~View();

        /** 
         * Set the view using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const Wall& wall );
        
        /** @return the last specified view as a wall. */
        const Wall& getWall() const { return _wall; }

        /** 
         * Set the view using a projection description.
         * 
         * @param projection the projection description.
         */
        void setProjection( const Projection& projection );

        /** @return the last specified view as a projection. */
        const Projection& getProjection() const { return _projection; }

        /** @return the type of the latest specified view. */
        Type getCurrentType() const { return _current; }

        /** 
         * Set the eye separation. 
         * @warning eye base API will change in the future.
         */
        void setEyeBase( const float eyeBase );

        /** 
         * @warning eye base API will change in the future.
         * @return the eye separation.
         */
        float getEyeBase() const { return _eyeBase; }

        /** @return true if the view has to be committed. */
        virtual bool isDirty() const { return (_dirty != 0); }

        /** @warning will not be supported in the future. */
        void setName( const std::string& name );

        /** @warning will not be supported in the future. */
        const std::string& getName() const;

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }

        virtual void getInstanceData( net::DataOStream& os );
        virtual void pack( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );

        enum DirtyBits
        {
            DIRTY_NONE       = 0,
            DIRTY_WALL       = 1 << 0,
            DIRTY_PROJECTION = 1 << 1,
            DIRTY_EYEBASE    = 1 << 2,
            DIRTY_NAME       = 1 << 3,
            DIRTY_ALL        = 0xffff
        };

        uint32_t _dirty;

    private:
        Wall        _wall;
        Projection  _projection;
        Type        _current;
        float       _eyeBase;
        std::string _name;

        /** worker for pack and getInstanceData() */
        void serialize( net::DataOStream& os, const uint32_t dirtyBits );
        /** worker for View( is ) constructor, unpack and applyInstanceData() */
        void deserialize( net::DataIStream& is );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const View& view );
}

#endif //EQ_VIEW_H
