
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRUSTUM_H
#define EQ_FRUSTUM_H

#include <eq/client/projection.h> // member
#include <eq/client/wall.h>       // member
#include <eq/net/object.h>        // base class

namespace eq
{
    class Frustum : public net::Object
    {
    public:
        /** Construct a new Frustum. */
        EQ_EXPORT Frustum();
        
        /** Destruct the frustum. */
        EQ_EXPORT virtual ~Frustum();

        /** The type of the latest specified frustum. */
        enum Type
        {
            TYPE_NONE,        //!< No frustum has been specified
            TYPE_WALL,        //!< A wall description has been set last
            TYPE_PROJECTION   //!< A projection description has been set last
        };

        /** Set the frustum using a wall description. */
        EQ_EXPORT void setWall( const Wall& wall );
        
        /** Set the frustum using a projection description. */
        EQ_EXPORT void setProjection( const Projection& projection );

        /** @return the last specified frustum as a wall. */
        EQ_EXPORT const Wall& getWall() const { return _wall; }

        /** @return the last specified frustum as a projection. */
        EQ_EXPORT const Projection& getProjection() const { return _projection; }

        /** @return the type of the latest specified frustum. */
        EQ_EXPORT Type getCurrentFrustum() const { return _current; }

    protected:
        /** @return true if the view has to be committed. */
        EQ_EXPORT virtual bool isDirty() const { return (_dirty != DIRTY_NONE); }

        /** 
         * Worker for pack() and getInstanceData().
         *
         * Override this and deserialize() if you 'subclass' the dirty
         * bits. Otherwise override the net::Object serialization methods and
         * isDirty(), which all should call the parent implementation.
         *
         * This method is called with DIRTY_ALL from getInstanceData() and with
         * the actual _dirty bits from pack(), which also resets the _dirty flag
         * afterwards. The dirty bits are transmitted beforehand.
         */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint32_t dirtyBits );

        /** 
         * Worker for unpack() and applyInstanceData().
         * 
         * This function is called with the dirty bits send by the master
         * instance. The dirty bits are received beforehand.
         * 
         * @sa serialize()
         */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint32_t dirtyBits );

        EQ_EXPORT virtual void getInstanceData( net::DataOStream& os );
        EQ_EXPORT virtual void pack( net::DataOStream& os );
        EQ_EXPORT virtual void applyInstanceData( net::DataIStream& is );

        /** Do not override. */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        /** The changed parts of the frustum since the last pack(). */
        enum DirtyBits
        {
            DIRTY_NONE       = 0,
            DIRTY_WALL       = 1 << 0,
            DIRTY_PROJECTION = 1 << 1,
            DIRTY_CUSTOM     = 1 << 2,
            DIRTY_ALL        = 0xffffffffu
        };

        /** 
         * The current dirty bits. Enable your own custom bits when changing
         * data.
         */
        uint32_t _dirty;

    private:
        /** The frustum description as a wall. */
        Wall _wall;

        /** The frustum description as a projection. */
        Projection _projection;

        /** The type of the last specified frustum description. */
        Type _current;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Frustum& );
}
#endif // EQ_FRUSTUM_H
