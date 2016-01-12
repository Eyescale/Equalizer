
/* Copyright (c) 2012-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_FRAME_H
#define EQFABRIC_FRAME_H

#include <eq/fabric/api.h>
#include <eq/fabric/eye.h>    // enum
#include <eq/fabric/types.h>
#include <co/object.h>  // base class

namespace eq
{
namespace fabric
{
namespace detail { class Frame; }

/** A holder for a frame data and related parameters. */
class Frame : public co::Object
{
public:
    /**
     * Components of the frame are to be used during readback and assembly.
     * @version 1.0
     */
    enum Buffer
    {
        BUFFER_NONE      = LB_BIT_NONE,
        BUFFER_UNDEFINED = LB_BIT1,  //!< Inherit, only if no others are set
        BUFFER_COLOR     = LB_BIT5,  //!< Use color images
        BUFFER_DEPTH     = LB_BIT9,  //!< Use depth images
        BUFFER_ALL       = LB_BIT_ALL_32
    };

    /** The storage type for pixel data. @version 1.0 */
    enum Type
    {
        TYPE_MEMORY,    //!< use main memory to store pixel data
        TYPE_TEXTURE    //!< use a GL texture to store pixel data
    };

    /** Construct a new frame. @version 1.0 */
    EQFABRIC_API Frame();

    /** Destruct the frame. @version 1.0 */
    EQFABRIC_API virtual ~Frame();

    /** @name Data Access */
    //@{
    /** Set the name of the frame. @version 1.3.3 */
    EQFABRIC_API void setName( const std::string& name );

    /** @return the name of the frame. @version 1.0 */
    EQFABRIC_API const std::string& getName() const;

    /** @return the position of the frame wrt the channel. @version 1.0 */
    EQFABRIC_API const Vector2i& getOffset() const;

    /**
     * Set the position of the frame wrt the channel.
     *
     * The offset is only applied for operations on this frame holder, i.e., it
     * does not apply to other (input) frames using the same underlying frame
     * data.
     * @version 1.0
     */
    EQFABRIC_API void setOffset( const Vector2i& offset );

    /**
     * Set the zoom for this frame holder.
     *
     * The zoom is only applied for operations on this frame holder, i.e., it
     * does not apply to other (input) frames using the same underlying frame
     * data.
     * @version 1.0
     */
    EQFABRIC_API void setZoom( const Zoom& zoom );

    /** @return the zoom factor for readback or assemble. @version 1.0 */
    EQFABRIC_API const Zoom& getZoom() const;

    /** @internal */
    EQFABRIC_API const co::ObjectVersion& getDataVersion( const Eye ) const;
    //@}

    /** @internal @return the receiving eq::Node IDs of an output frame */
    EQFABRIC_API
    const std::vector< uint128_t >& getInputNodes( const Eye eye ) const;

    /** @internal @return the receiving co::Node IDs of an output frame */
    EQFABRIC_API const co::NodeIDs& getInputNetNodes(const Eye eye) const;

protected:
    virtual ChangeType getChangeType() const { return INSTANCE; }
    EQFABRIC_API virtual void getInstanceData( co::DataOStream& os );
    EQFABRIC_API virtual void applyInstanceData( co::DataIStream& is );

    /** @internal */
    EQFABRIC_API void _setDataVersion( const unsigned i,
                                       const co::ObjectVersion& ov );

    /** @internal @return the receiving eq::Node IDs of an output frame */
    EQFABRIC_API std::vector< uint128_t >& _getInputNodes( const unsigned i );

    /** @internal @return the receiving co::Node IDs of an output frame */
    EQFABRIC_API co::NodeIDs& _getInputNetNodes( const unsigned i );

private:
    detail::Frame* const _impl;
};

/** Print the frame to the given output stream. @version 1.4 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const Frame& );
/** Print the frame type to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << ( std::ostream&, const Frame::Type );
/** Print the frame buffer value to the given output stream. @version 1.0 */
EQFABRIC_API std::ostream& operator << (std::ostream&, const Frame::Buffer);
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Frame::Type& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
#endif // EQFABRIC_FRAME_H
