
#ifndef MASS_VOL__VMML_TYPES
#define MASS_VOL__VMML_TYPES

#include <eq/fabric/types.h>


namespace massVolVis
{

using eq::fabric::Matrix4f;
using eq::fabric::Matrix4d;
using eq::fabric::Matrix3f;

using eq::fabric::Vector4f;
using eq::fabric::Vector3f;
using eq::fabric::Vector2f;

using eq::fabric::Vector4d;

typedef vmml::vector< 2, uint32_t > Vector2ui;

/*
typedef vmml::matrix< 3, 3, double > Matrix3d;
typedef vmml::matrix< 4, 4, double > Matrix4d;

typedef vmml::vector< 3,    double > Vector3d;
typedef vmml::vector< 4,    float  > Vector4f;

typedef vmml::vector< 2,  uint32_t > Vector2ui;
*/
}

#endif //MASS_VOL__VMML_TYPES