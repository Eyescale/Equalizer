
#include <eq/eq.h>


#ifndef EQ_VOL_SLICECLIPPING_H
#define EQ_VOL_SLICECLIPPING_H

namespace eVolve
{


struct SliceClipper
{
    SliceClipper(){};

    static const int    nSequence[8][8];
    static const float  sequence[64];
    static const float  v1[24];
    static const float  v2[24];

    typedef vmml::Vector3f float3;

    void updatePerFrameInfo
    (
        const vmml::Matrix4d&   modelviewM,
        const vmml::Matrix3d&   modelviewITM,
        const double            sliceDistance,
        const eq::Range&        range
    );

    vmml::Vector3f getPosition
    (
        const int vertexNum,
        const int sliceNum
    ) const;

    float           shaderVertices[24];
    vmml::Vector3f  viewVecf;
    vmml::Vector4d  viewVec;
    int             frontIndex;
    double          sliceDistance;
    double          planeStart;
};

}

#endif //EQ_VOL_SLICECLIPPING_H
