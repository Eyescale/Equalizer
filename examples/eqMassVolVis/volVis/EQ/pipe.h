
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2012, David Steiner   <steiner@ifi.uzh.ch>
 */

#ifndef MASS_VOL__PIPE_H
#define MASS_VOL__PIPE_H

#include <eq/eq.h>

#include "frameData.h"


namespace massVolVis
{

class Window;
class Model;

typedef boost::shared_ptr< Model > ModelSPtr;


/**
 * Pipe is a standard EQ abstraction for GPUs. Each pipe creates a Model that
 * is shared by all windows. A Model handles all communication with a GPU loader.
 */
class Pipe : public eq::Pipe
{
public:
    Pipe( eq::Node* parent );

    virtual ~Pipe(){}

    const FrameData& getFrameData() const { return _frameData; }

    /**
     * Set renderer / adjust transfer functions if it changes.
     * Triggered from Window::frameStart
     */
    void checkRenderingParameters( Window* wnd, uint32_t frameNumber );

    ModelSPtr getModel(){ return _model; }

protected:

    /**
     * Is called when data file is changed. Creates a model which causes all async
     * stuff, that requires correct OpenGL context of a window, to start.
     */
    void _intializeModel( Window* wnd );

    /**
     * Destroys the model, which stopes all async operations. Is called automatically
     * from configExit.
     */
    void _deintializeModel();

    /**
     */
    virtual void frameStart(  const eq::uint128_t& frameId, const uint32_t frameNumber );

    virtual bool configInit( const eq::uint128_t& initId );
    virtual bool configExit();

    virtual eq::WindowSystem selectWindowSystem() const;

private:
    FrameData _frameData;

// New version of TF, Renderer of Model means user is asking to change the 
// TF, Renderer or Data set
    ModelSPtr _model;           //!< model is shared per pipe

    uint32_t  _rTypeVersion;    //!< version of the renderer type
    uint32_t  _tfVersion;       //!< version of the TF
    uint32_t  _fileNameVersion; //!< version of the model name (data set version)

    uint32_t _lastFrameNumber; //!< used to make sure we check for new parameters only once per frame
};



}//namespace massVolVis

#endif //MASS_VOL__PIPE_H


