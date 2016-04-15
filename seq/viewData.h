
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_VIEWDATA_H
#define EQSEQUEL_VIEWDATA_H

#include <seq/api.h>
#include <seq/types.h>
#include <eq/fabric/vmmlib.h>
#include <co/serializable.h>               // base class

namespace seq
{
/** Stores per-view data. */
class ViewData : public co::Serializable
{
public:
    /** Construct a new view data. @version 1.0 */
    SEQ_API explicit ViewData( View& view );

    /** Destruct this view data. @version 1.0 */
    SEQ_API virtual ~ViewData();

    /** @name Operations */
    //@{
#ifndef EQ_2_0_API
    /**
     * Handle the given event.
     *
     * The default implementation provides a pointer-based camera model and some
     * key event handling, all of which can be modified by overwriting this
     * method and handling the appropriate events.
     * @version 1.0
     * @deprecated
     */
    SEQ_API virtual bool handleEvent( const eq::ConfigEvent* event );
#endif

    /**
     * Handle the given event command.
     *
     * The default implementation provides a pointer-based camera model and some
     * key event handling, all of which can be modified by overwriting this
     * method and handling the appropriate events.
     * @version 1.5.1
     */
    SEQ_API virtual bool handleEvent( eq::EventICommand command );

    /** Rotate the model matrix by the given increments. @version 1.0 */
    SEQ_API void spinModel( const float x, const float y, const float z );

    /** Move the model matrix by the given increments. @version 1.0 */
    SEQ_API void moveModel( const float x, const float y, const float z );

    /**
     * Enable or disable statistics rendering.
     *
     * The statistics are rendered in the views where they are enabled. The
     * default event handler of this view toggles the statistics rendering state
     * when 's' is pressed.
     *
     * @param on the state of the statistics rendering.
     * @version 1.0
     */
    SEQ_API void showStatistics( const bool on );

    /**
     * Enable or disable orthographic rendering.
     *
     * The default event handler of this view toggles the orthographic rendering
     * state when 'o' is pressed.
     *
     * @param on the state of the orthographic rendering.
     * @version 1.2
     */
    SEQ_API void setOrtho( const bool on );

    /**
     * Update the view data.
     *
     * Called once at the end of each frame to trigger animations. The default
     * implementation updates the camera data.
     *
     * @return true to request a redraw.
     * @version 1.0
     */
    SEQ_API virtual bool update();
    //@}

    /** @name Data Access. */
    //@{
    /** Set the current model matrix (global camera). @version 1.11 */
    SEQ_API void setModelMatrix( const Matrix4f& matrix );

    /** @return the current model matrix (global camera). @version 1.0 */
    const Matrix4f& getModelMatrix() const { return _modelMatrix; }

    /** @return true is statistics are rendered. @version 1.0 */
    bool getStatistics() const { return _statistics; }
    //@}

    /** @return true when orthographic rendering is enabled. @version 1.2 */
    bool useOrtho() const { return _ortho; }
    //@}

protected:
    SEQ_API void serialize( co::DataOStream& os, const uint64_t dirtyBits ) override;
    SEQ_API void deserialize( co::DataIStream& is,
                              const uint64_t dirtyBits ) override;

    /** The changed parts of the object since the last serialize(). */
    enum DirtyBits
    {
        DIRTY_MODELMATRIX = co::Serializable::DIRTY_CUSTOM << 0, // 1
        DIRTY_STATISTICS = co::Serializable::DIRTY_CUSTOM << 1, // 2
        DIRTY_ORTHO = co::Serializable::DIRTY_CUSTOM << 2, // 4
        DIRTY_CUSTOM = co::Serializable::DIRTY_CUSTOM << 3 // 8
    };

private:
    bool _handleEvent( const eq::Event& event );

    View& _view;
    Matrix4f _modelMatrix;
    int32_t _spinX, _spinY;
    int32_t _advance;
    bool _statistics;
    bool _ortho;
};
}
#endif // EQSEQUEL_VIEWDATA_H
