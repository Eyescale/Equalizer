
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 */

#ifndef MASS_VOL__RENDERER_TYPES_H
#define MASS_VOL__RENDERER_TYPES_H

namespace massVolVis
{

/**
 * Avaliable renderers.
 */
enum RendererType
{
    NONE    = 0,
    SLICE   = 1,
    RAYCAST = 2
};

} //namespace massVolVis

namespace lunchbox
{
template<> inline void byteswap( massVolVis::RendererType& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}

#endif //MASS_VOL__RENDERER_TYPES_H

