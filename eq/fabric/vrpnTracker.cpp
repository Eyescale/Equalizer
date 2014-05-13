
/* Copyright (c) 2014-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "vrpnTracker.h"

#include <string>
#include <map>

#include <boost/regex.hpp>
#include <boost/assign.hpp>

namespace eq
{
namespace fabric
{
std::ostream& operator << ( std::ostream& os,
                            const VRPNTrackerSensor::SensorID value )
{
    switch( value )
    {
        case VRPNTrackerSensor::UseAnyValidSensorID:    os << "ANY"; break;
        case VRPNTrackerSensor::UseLowestValidSensorID: os << "LOWEST"; break;
        default:        os << static_cast< int >( value );
    }
    return os;
}


VRPNTrackerAxis::VRPNTrackerAxis() :
    _transform( Matrix4f::IDENTITY )
{
    _transform(2,2) = -1.f; // TODO: remove old default; specify explicitly
}

VRPNTrackerAxis::VRPNTrackerAxis(const VRPNTrackerAxis& axis)
{
    _transform = axis._transform;
}

VRPNTrackerAxis::VRPNTrackerAxis(   const Vector3f& x,
                                    const Vector3f& y,
                                    const Vector3f& z) :
    _transform()
{
    _transform.set_row(0, x);
    _transform.set_row(1, y);
    _transform.set_row(2, z);
}

VRPNTrackerAxis::VRPNTrackerAxis(const std::string& axis) :
    _transform( Matrix4f::IDENTITY )
{
    _transform(2,2) = -1.f; // TODO: remove old default; specify explicitly

    if (!axis.empty())
    {
        if (!parseTransform(axis, _transform))
            LBWARN << "VRPN tracker couldn't process axis option: " << axis;
    }
}

bool VRPNTrackerAxis::operator==(const VRPNTrackerAxis& axis ) const
{
    return _transform == axis._transform;
}

bool VRPNTrackerAxis::parseTransform(const std::string& axis,
                           Matrix3f& transformOut )
{
    static const boost::regex k_axis_re("^([xyzXYZ])([xyzXYZ])([xyzXYZ])$");
    static std::map<char,Vector3f> k_axisMap =
            boost::assign::map_list_of ('X', Vector3f::UNIT_X)
                                       ('Y', Vector3f::UNIT_Y)
                                       ('Z', Vector3f::UNIT_Z);

    boost::smatch optResult;
    if (boost::regex_search(axis, optResult, k_axis_re))
    {
        for (size_t r = 0; r < transformOut.ROWS; ++r)
        {
            std::string optionValue(optResult.str(r+1));
            LBASSERT(optionValue.size() == 1);
            const char ax = optionValue[0];
            const float scale = std::isupper(ax) ? 1.f : -1.f;
            const Vector3f v = scale * k_axisMap[std::toupper(ax)];

            transformOut.set_row(r, v);
        }
    }
    else
        return false;

    return true;
}

}
}
