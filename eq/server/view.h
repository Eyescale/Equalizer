
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_VIEW_H
#define EQSERVER_VIEW_H

#include <eq/server/api.h>
#include "types.h"

#include <eq/fabric/view.h>     // base class
#include <eq/fabric/viewport.h> // member

namespace eq
{
namespace server
{
class View : public fabric::View< Layout, View, Observer >
{
public:
    EQSERVER_API explicit View( Layout* parent );

    virtual ~View();

    /** @name Data Access. */
    //@{
    /** @return the config of this view. */
    Config* getConfig();

    /** @return the config of this view. */
    const Config* getConfig() const;

    /** @return the Server of this view. */
    ServerPtr getServer();

    /** @return the index path to this view. */
    ViewPath getPath() const;

    /**
     * Adds a new destination channel to this view.
     *
     * @param channel the channel.
     */
    void addChannel( Channel* channel );

    /**
     * Removes a destination channel from this view.
     *
     * @param channel the channel
     * @return <code>true</code> if the channel was removed,
     *         <code>false</code> otherwise.
     */
    bool removeChannel( Channel* channel );

    /** @return the vector of destination channels. */
    const Channels& getChannels() const { return _channels; }

    /** @name Operations */
    //@{
    /** Initialize the view parameters. */
    void init();

    /**
     * Trigger a view (de)activation.
     *
     * @param canvas The canvas to activate, or 0 to activate for all
     *               canvases using this view's layout.
     * @param active true to activate, false to deactivate.
     */
    void trigger( const Canvas* canvas, const bool active );

    /**
     * Activate the given mode on this view.
     *
     * @param mode the new rendering mode
     */
    void activateMode( const Mode mode ) override;

    void updateCapabilities() override;

    /** Update all segment frusta based on the current settings. */
    void updateFrusta();

    void setSAttribute( const SAttribute attr, const std::string& value )
    { fabric::View< Layout, View, Observer >::setSAttribute( attr, value ); }

    void setDirty( const uint64_t bits ) override; //!< @internal
    //@}

protected:
    /** @internal */
    void deserialize( co::DataIStream&, const uint64_t ) override;
    void notifyAttached() override { _updateChannels(); }

private:
    /** The list of channels. */
    Channels _channels;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    void _updateChannels() const;
    float _computeFocusRatio( Vector3f& eye );
};
}
}
#endif // EQSERVER_VIEW_H
