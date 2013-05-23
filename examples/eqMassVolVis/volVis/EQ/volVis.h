
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2012, David Steiner   <steiner@ifi.uzh.ch>
 */

#ifndef MASS_VOL__VOL_VIS_H
#define MASS_VOL__VOL_VIS_H

#include <eq/eq.h>

namespace massVolVis
{

class LocalInitData;

class VolVis : public eq::Client
{
public:
    VolVis( const LocalInitData& initData );
    virtual ~VolVis();

    /** Run a VolVis instance. */
    int run();

    static const std::string& getHelp();

protected:
    /** @sa eq::Client::clientLoop. */
    virtual void clientLoop();

private:
    const LocalInitData& _initData;
};

enum ColorMode
{
    COLOR_MODEL,    //!< Render using the colors defined in the ply file
    COLOR_DEMO,     //!< Use a unique color to demonstrate decomposition
    COLOR_HALF_DEMO,//!< 50% unique color + 50% original color
    COLOR_NORMALS,  //!< replace colors with values of normals
    COLOR_ALL       //!< @internal, must be last
};

enum BackgroundMode
{
    BG_BLACK,   //!< Black background
    BG_WHITE,   //!< White background
    BG_COLOR,   //!< Unique color
    BG_ALL      //!< @internal, must be last
};

enum NormalsQuality
{
    NQ_FULL,    //!< Highest normals quality
    NQ_MEDIUM,  //!< Average normals quality
    NQ_MINIMAL, //!< Basic normal approximation
    NQ_ALL      //!< @internal, must be last
};

enum LogTopics
{
    LOG_STATS = eq::LOG_CUSTOM      // 65536
};
}//namespace massVolVis

namespace lunchbox
{
template<> inline void byteswap( massVolVis::ColorMode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }

template<> inline void byteswap( massVolVis::BackgroundMode& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }

template<> inline void byteswap( massVolVis::NormalsQuality& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
#endif // MASS_VOL__VOL_VIS_H

