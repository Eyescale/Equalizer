
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

#ifndef EQFABRIC_VRPNTRACKER_H
#define EQFABRIC_VRPNTRACKER_H

#include <eq/fabric/api.h>
#include <eq/fabric/vmmlib.h>
#include <lunchbox/debug.h>
#include <lunchbox/bitOperation.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /** Special VRPN Sensor ID values */
    struct VRPNTrackerSensor
    {
        enum SensorID
        {
            UseAnyValidSensorID = -1,
            UseLowestValidSensorID = -2
        };
    };

    EQFABRIC_API std::ostream& operator << ( std::ostream& os,
                                const VRPNTrackerSensor::SensorID value );

    /** Specification of an axis swizzle. */
    class VRPNTrackerAxis
    {
    public:

        /** @name Constructors */
        //@{
        /** Construct a new default axis transform. */
        VRPNTrackerAxis();

        /** Copy Construct a new axis from existing axis. */
        VRPNTrackerAxis(const VRPNTrackerAxis& axis);

        /** Construct a new axis transform from X Y Z vectors. */
        VRPNTrackerAxis(const Vector3f& x, const Vector3f& y, const Vector3f& z);

        /** Construct a new axis transform from string representation. */
        VRPNTrackerAxis(const std::string& axis);

        //@}

        /** @name Data Access */
        //@{
        /** @return return coordinate transform. */
        const Matrix3f& getTransform() const { return _transform; }
        /** @return return coordinate transform. */
        Matrix3f& getTransform() { return _transform; }

        bool operator==(const VRPNTrackerAxis& axis ) const;

        //@}

        /** @name Utilities */
        //@{
        /** @return parse axis string into transform; return true if
         * successful (transform fully initialized), false otherwise.
         */
        static bool parseTransform(const std::string& axis,
                                   Matrix3f& transformOut );

        //@}

    protected:

        Matrix3f _transform;        //!< Axis transform
    };


    inline std::ostream& operator << ( std::ostream& os,
                                       const VRPNTrackerAxis& axis )
    {
        const Matrix3f& t(axis.getTransform());

        os << "[ "
           << "[ " << t(0,0) << " " << t(0,1) << " " << t(0,2) << "] "
           << "[ " << t(1,0) << " " << t(1,1) << " " << t(1,2) << "] "
           << "[ " << t(2,0) << " " << t(2,1) << " " << t(2,2) << "] "
           << "]";
        return os;
    }
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::VRPNTrackerAxis& value )
{
    byteswap( value.getTransform() );
}
}
#endif // EQFABRIC_VRPNTRACKER_H
