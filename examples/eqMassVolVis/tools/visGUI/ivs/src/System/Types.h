/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_TYPES_H
#define IVS_SYSTEM_TYPES_H

#include <eq/fabric/vmmlib.h>

namespace ivs
{
// standard vector and matrix typedefs
typedef vmml::vector<2, unsigned char> Vec2u;
typedef vmml::vector<2, int>           Vec2i;
typedef vmml::vector<2, unsigned int>  Vec2ui;
typedef vmml::vector<2, float>         Vec2f;
typedef vmml::vector<2, double>        Vec2d;
typedef vmml::vector<3, char>          Vec3c;
typedef vmml::vector<3, unsigned char> Vec3u;
typedef vmml::vector<3, int>           Vec3i;
typedef vmml::vector<3, unsigned int>  Vec3ui;
typedef vmml::vector<3, float>         Vec3f;
typedef vmml::vector<3, double>        Vec3d;
typedef vmml::vector<4, unsigned char> Vec4u;
typedef vmml::vector<4, int>           Vec4i;
typedef vmml::vector<4, unsigned int>  Vec4ui;
typedef vmml::vector<4, float>         Vec4f;
typedef vmml::vector<4, double>        Vec4d;
typedef vmml::matrix<3, 3, int>        Matrix3i;
typedef vmml::matrix<3, 3, float>      Matrix3f;
typedef vmml::matrix<3, 3, double>     Matrix3d;
typedef vmml::matrix<4, 4, int>        Matrix4i;
typedef vmml::matrix<4, 4, float>      Matrix4f;
typedef vmml::matrix<4, 4, double>     Matrix4d;
typedef vmml::quaternion<float>        Quaternionf;
typedef vmml::quaternion<double>       Quaterniond;
}

#endif
