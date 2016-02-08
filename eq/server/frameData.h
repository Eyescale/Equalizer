
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_FRAMEDATA_H
#define EQSERVER_FRAMEDATA_H

#include <eq/fabric/frame.h>      // for Frame::Type
#include <eq/fabric/frameData.h>  // member
#include <eq/server/types.h>

namespace eq
{
namespace server
{
/** A holder for a Frame Data and parameters. */
class FrameData : public fabric::FrameData, public co::Object
{
public:
    /** Construct a new FrameData. */
    FrameData();
    virtual ~FrameData(){}

    /** @name Data Access */
    //@{
    /** Set the number of the frame when this data was last used. */
    void setFrameNumber( const uint32_t number ) { _frameNumber = number; }
    uint32_t getFrameNumber() const { return _frameNumber; }

    /** Set the position of the data relative to the window. */
    void setOffset( const Vector2i& offset ) { _offset = offset; }
    /** @return the position of the data relative to the window. */
    const Vector2i& getOffset() const { return _offset; }

    /** Set the output frame zoom factor. */
    void setZoom( const Zoom& zoom_ ) { _zoom = zoom_; }
    const Zoom& getZoom() const { return _zoom; }
    //@}

protected:
    virtual ChangeType getChangeType() const { return INSTANCE; }
    virtual void getInstanceData( co::DataOStream& os );
    virtual void applyInstanceData( co::DataIStream& is );

private:
    /** The zoom factor of the output frame after readback. */
    Zoom _zoom;

    /** Position wrt destination view. */
    Vector2i _offset;

    /** The number of the config frame when this data was last used. */
    uint32_t _frameNumber;
};
}
}
#endif // EQSERVER_FRAMEDATA_H
