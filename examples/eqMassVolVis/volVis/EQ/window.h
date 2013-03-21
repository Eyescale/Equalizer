
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOL__WINDOW_H
#define MASS_VOL__WINDOW_H

#include <eq/eq.h>


namespace massVolVis
{


/*  */
class Window : public eq::Window
{
public:
    Window( eq::Pipe* parent ) : eq::Window( parent ) {}

    const eq::util::Texture* getLogoTexture() const { return _logoTexture; }

protected:
    virtual void swapBuffers();
    virtual bool configInit(    const eq::uint128_t& initId );
    virtual bool configInitGL(  const eq::uint128_t& initId );

    /**
     * Ask pipe to check renderer and TF parameters.
     */
    virtual void frameStart(  const eq::uint128_t& frameId, const uint32_t frameNumber );

public:
    eq::util::Texture* _logoTexture;

    void _loadLogo();
};





}//namespace massVolVis

#endif //MASS_VOL__WINDOW_H


