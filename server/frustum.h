
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_FRUSTUM_H
#define EQSERVER_FRUSTUM_H

#include <eq/client/frustum.h>     // base class

namespace eq
{
namespace server
{
    struct FrustumData;

    /** 
     * Extends the eq::Frustum to update server-side generic frustum data.
     */
    class Frustum : public eq::Frustum
    {
    public:
        Frustum( FrustumData& data );
        Frustum( const Frustum& from, FrustumData& data );
        virtual ~Frustum(){}
        
        /** Set the frustum using a wall description. */
        void setWall( const eq::Wall& wall );
        
        /** Set the frustum using a projection description. */
        void setProjection( const eq::Projection& projection );

        /** Set the eye separation. */
        void setEyeBase( const float eyeBase );

        /** Update the eye positions based on the head matrix. */
        void updateHead();

    protected:

    private:
        FrustumData& _data;

        float _eyeBase;

        /** Update the frustum (wall/projection). */
        void _updateFrustum();
    };

    std::ostream& operator << ( std::ostream& os, const Frustum* frustum );
}
}
#endif // EQ_FRUSTUM_H
