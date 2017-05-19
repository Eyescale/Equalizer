
/* Copyright (c) 2011-2017, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_DETAIL_MASTERCONFIG_H
#define EQSEQUEL_DETAIL_MASTERCONFIG_H

#include "config.h" // base class

namespace seq
{
namespace detail
{
class MasterConfig : public Config
{
public:
    explicit MasterConfig(eq::ServerPtr parent);

    bool init() final;
    bool run(co::Object* frameData) final;
    bool exit() final;

    bool needRedraw() final { return _redraw; }
    uint32_t startFrame() final;

protected:
    virtual ~MasterConfig();

private:
    uint128_t _currentViewID;
    bool _redraw;

    bool handleEvent(EventICommand command) final;
    bool handleEvent(eq::EventType type, const SizeEvent& event) final;
    bool handleEvent(eq::EventType type, const PointerEvent& event) final;
    bool handleEvent(eq::EventType type, const KeyEvent& event) final;
    bool handleEvent(const AxisEvent& event) final;
    bool handleEvent(const ButtonEvent& event) final;
    bool handleEvent(eq::EventType type, const Event& event) final;
    void addStatistic(const Statistic& stat) final;
    template <class E>
    bool _handleEvent(eq::EventType type, E& event);
    template <class E>
    bool _handleEvent(E& event);
};
}
}

#endif // EQSEQUEL_DETAIL_MASTERCONFIG_H
