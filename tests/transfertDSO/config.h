
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PIXELBENCH_CONFIG_H
#define EQ_PIXELBENCH_CONFIG_H

#include <eq/eq.h>

/** The Equalizer Pixel Transfer Benchmark Utility */
namespace eqTransfertDSO
{
class Config : public eq::Config
{
public:
    Config( eq::base::RefPtr< eq::Server > parent );

    /** @sa eq::Config::startFrame */
    virtual uint32_t startFrame( const uint32_t frameID );

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( const eq::ConfigEvent* event );

    /** @return the clock started by startFrame, or 0 on render clients. */
    const eq::base::Clock* getClock() const { return _clock; }

protected:
    virtual ~Config();

private:
    eq::base::Clock* _clock;
};
}

#endif // EQ_PIXELBENCH_CONFIG_H
