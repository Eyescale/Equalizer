
/* Copyright (c) 2011-2019, Stefan Eilemann <eile@eyescale.h>
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

#ifndef EQSERVER_CONFIG_RESOURCES_H
#define EQSERVER_CONFIG_RESOURCES_H

#include "../types.h"

#define EQ_SERVER_CONFIG_LAYOUT_SIMPLE "Simple"
#define EQ_SERVER_CONFIG_LAYOUT_2D_STATIC "Static2D"
#define EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC "Dynamic2D"
#define EQ_SERVER_CONFIG_LAYOUT_DB_STATIC "StaticDB"
#define EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC "DynamicDB"
#define EQ_SERVER_CONFIG_LAYOUT_DB_DS "DBDirectSend"
#define EQ_SERVER_CONFIG_LAYOUT_DB_2D "DB_2D"
#define EQ_SERVER_CONFIG_LAYOUT_SUBPIXEL "Subpixel"
#define EQ_SERVER_CONFIG_LAYOUT_TILES "Tiles"
#define EQ_SERVER_CONFIG_LAYOUT_CHUNKS "Chunks"
#define EQ_SERVER_CONFIG_LAYOUT_DPLEX "DPlex"
#define EQ_SERVER_CONFIG_LAYOUT_WALL "Wall"
#define EQ_SERVER_COMPOUND_CLEAR "clear"

namespace eq
{
namespace server
{
namespace config
{
class Resources
{
public:
    static bool discover(ServerPtr server, Config* config,
                         const std::string& session,
                         const fabric::ConfigParams& params);
    static Channels configureSourceChannels(Config* config);
    static void configure(const Compounds& compounds, const Channels& channels,
                          const fabric::ConfigParams& params);
    static void configureWall(Config* config, const Channels& channels);
};

enum class DemoMode
{
    none,
    fullscreen,
    window
};

static const DemoMode demoMode = []() {
    const auto env = (getenv("EQ_SERVER_CONFIG_DEMOMODE"));
    if (!env)
        return DemoMode::none;

    const std::string mode(env);
    if (env == std::string("window"))
        return DemoMode::window;

    return DemoMode::fullscreen;
}();
}
}
}
#endif // EQSERVER_CONFIG_RESOURCES_H
