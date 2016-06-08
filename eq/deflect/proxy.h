
/* Copyright (c) 2013-2015, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef EQ_DEFLECT_PROXY_H
#define EQ_DEFLECT_PROXY_H

#include <eq/resultImageListener.h> // base class
#include <eq/types.h>
#include <deflect/types.h>

namespace eq
{
namespace deflect
{
/** @internal */
class Proxy : public ResultImageListener
{
public:
    /** Construct a Deflect proxy associated to a destination channel. */
    explicit Proxy( Channel& channel );

    /** Destruct the Deflect proxy. */
    ~Proxy();

    /** Stream the given image to the Deflect host. */
    void notifyNewImage( Channel& channel, const Image& image ) final;

    /** @return the associated destination channel. */
    Channel& getChannel();

    /** @return the underlying socket descriptor. */
    int getSocketDescriptor() const;

    /** @return true if a new Event was sent by the Deflect host. */
    bool hasNewEvent() const;

    /** @return whether the application is running on the Deflect host. */
    bool isRunning() const;

    /**
     * Signal the proxy to stop running.
     *
     * This is called if the application is closed in the Deflect host.
     */
    void stopRunning();

    /** @return the latest window Event. @sa hasNewEvent() */
    ::deflect::Event getEvent() const;

    enum NavigationMode
    {
        MODE_PAN,
        MODE_ROTATE
    };

    /** Set the navigation mode for received mouse events. */
    void setNavigationMode( NavigationMode mode );

    /** @return the current set navigation mode. */
    NavigationMode getNavigationMode() const;

    /** @return a help text which can be printed as an overlay. */
    std::string getHelp() const;

private:
    class Impl;
    std::unique_ptr< Impl > _impl;
};
}
}
#endif // EQ_DC_PROXY_H
