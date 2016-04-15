Changelog {#Changelog}
=========

# git master

* [554](https://github.com/Eyescale/Equalizer/pull/554):
  Expose current view data in seq::Renderer  
* [548](https://github.com/Eyescale/Equalizer/pull/548):
  Sequel tweaks for seqSplotch

  * configInit()/configExit() for eq::View
  * seq::Application::createViewData receives eq::View as parameter
  * seq::Renderer::createViewData receives eq::View as parameter
  * seq::ViewData considers eq::View::getModelUnit for model matrix manipulation


# Release 1.11 (21-Mar-2016)

* [542](https://github.com/Eyescale/Equalizer/pull/542):
  Fix missing/wrong handling of key events from Deflect host
* [538](https://github.com/Eyescale/Equalizer/pull/538):
  Support sorted image-based compositing. Minor API changes in the
  compositor, frame and image code
* [532](https://github.com/Eyescale/Equalizer/pull/532):
  Compositor::blendFrames() replaces Compositor::assembleFramesSorted()
* [529](https://github.com/Eyescale/Equalizer/pull/529):
  Pan/rotate mode switch for deflect::Proxy on tap & hold,
  add Channel::frameDrawOverlay()
* [529](https://github.com/Eyescale/Equalizer/pull/529):
  Add apply/resetOverlayState in Channel for 2D overlay rendering
* [527](https://github.com/Eyescale/Equalizer/pull/527):
  Fix Deflect wheel event to consider pixel delta from pinch
* [520](https://github.com/Eyescale/Equalizer/issues/520):
  Fix tile decomposition without ROI broken
* [518](https://github.com/Eyescale/Equalizer/pull/518):
  Enable customisation of client loop
* [511](https://github.com/Eyescale/Equalizer/pull/511):
  Fix exit behaviour on config parse error
* [510](https://github.com/Eyescale/Equalizer/issues/510):
  Fix async readback deadlock with direct send compositing

# Release 1.10 (5-Nov-2015)

* [508](https://github.com/Eyescale/Equalizer/pull/508):
  Post-divide alpha from pixels in eq::Image::writeImage()
* [504](https://github.com/Eyescale/Equalizer/pull/504):
  Let the OS choose the server port
* [502](https://github.com/Eyescale/Equalizer/pull/500):
  Fixed support of multi-pipe configurations in Qt
* [499](https://github.com/Eyescale/Equalizer/pull/499):
  Fixes for VirtualGL 2.4.80 with multi-pipe configurations
* [484](https://github.com/Eyescale/Equalizer/pull/484):
  Fix transfer window deadlock with Qt5
* [484](https://github.com/Eyescale/Equalizer/pull/484):
  Implement Window::doneCurrent() to make no OpenGL context current in the
  current thread
* [481](https://github.com/Eyescale/Equalizer/pull/481):
  Fix Config::getNextEvent() with definite timeout
* [467](https://github.com/Eyescale/Equalizer/issues/467):
  Fix CPU load with idle Qt-based windows
* [479](https://github.com/Eyescale/Equalizer/pull/479):
  Fix mouse wheel direction on AGL
* [476](https://github.com/Eyescale/Equalizer/issues/476):
  Fix slow readback of source channels with empty ROI
* [475](https://github.com/Eyescale/Equalizer/pull/475):
  Fix GL error on exit if frameWriter was used
* [473](https://github.com/Eyescale/Equalizer/pull/473):
  Fix OS X OpenGL detection
* [463](https://github.com/Eyescale/Equalizer/pull/463):
  Use program name as default window title
* [463](https://github.com/Eyescale/Equalizer/pull/463):
  Improved OpenGL and GLEW compatibility on OS X
* [463](https://github.com/Eyescale/Equalizer/pull/463):
  Fixed Qt5 support on OS X
* [463](https://github.com/Eyescale/Equalizer/pull/463):
  Standalone Qt5 support

# Release 1.9 (7-Jul-2015)

* [462](https://github.com/Eyescale/Equalizer/pull/462):
  Add eq::Client::addActiveLayout to tweak default autoconfigured layout
* [461](https://github.com/Eyescale/Equalizer/pull/461):
  Allow custom filename for SATTR_DUMP_IMAGE, use OSG to write different image
  formats
* [458](https://github.com/Eyescale/Equalizer/pull/458):
  Add byteswap for unsigned int vectors
* [453](https://github.com/Eyescale/Equalizer/pull/453),
  [440](https://github.com/Eyescale/Equalizer/pull/440):
  Port to Qt5 using new GL classes
* [447](https://github.com/Eyescale/Equalizer/pull/447):
  Sequel extensions for stardust
* [437](https://github.com/Eyescale/Equalizer/pull/437):
  System window and example for CPU-based rendering.
* [434](https://github.com/Eyescale/Equalizer/pull/434):
  #[60](https://github.com/Eyescale/Equalizer/issues/60):
  Multisample FBO support
* [431](https://github.com/Eyescale/Equalizer/pull/431):
  VGL_EXCLUDE for non-display GPUs
* [429](https://github.com/Eyescale/Equalizer/pull/429):
  Remove channel FBO drawable; use window hint_drawable FBO instead
* [428](https://github.com/Eyescale/Equalizer/pull/428):
  Implement support for OpenGL core context creation
  #[156](https://github.com/Eyescale/Equalizer/issues/156)
* [426](https://github.com/Eyescale/Equalizer/pull/426):
  EqGLLibraries: export correct GLEW_MX lib
* [425](https://github.com/Eyescale/Equalizer/pull/425):
  Fix NPR in --eq-layout code
* [417](https://github.com/Eyescale/Equalizer/pull/417):
  Improve auto placement on dual-screen setups
* [415](https://github.com/Eyescale/Equalizer/pull/415):
  Add eq::ResultImageListener to intercept destination channel rendering
* [409](https://github.com/Eyescale/Equalizer/pull/409):
  Switch to separated Deflect library
* [407](https://github.com/Eyescale/Equalizer/pull/407),
  [405](https://github.com/Eyescale/Equalizer/pull/405),
  [399](https://github.com/Eyescale/Equalizer/pull/399):
  Denoise log outputs
* [405](https://github.com/Eyescale/Equalizer/pull/405),
  [392](https://github.com/Eyescale/Equalizer/pull/392):
  Fix async texture readback
* [400](https://github.com/Eyescale/Equalizer/pull/400):
  Allow sharing with external/non-EQ managed QGLWidgets
* [397](https://github.com/Eyescale/Equalizer/pull/397):
  Added ERROR_FRAMEBUFFER_INVALID_SIZE error when creating FBO with invalid size
* [395](https://github.com/Eyescale/Equalizer/pull/395):
  Fix semantics of Config::update()
* [394](https://github.com/Eyescale/Equalizer/pull/394):
  Allow set of entire window settings; can be useful to transfer settings in the
  admin side
* [392](https://github.com/Eyescale/Equalizer/pull/392):
  Add default connection for autoconfigured server
* [391](https://github.com/Eyescale/Equalizer/pull/391):
  Complete instantiations for eq::admin::Config::find<>
* [390](https://github.com/Eyescale/Equalizer/pull/390):
  Fix build w/ OpenCV
* [389](https://github.com/Eyescale/Equalizer/pull/389):
  Server library no longer depends on client library, use local server directly
  from server library rather than dlopen it
* [385](https://github.com/Eyescale/Equalizer/pull/385):
  Fix offscreen Qt window: viewport and transfer window
* [383](https://github.com/Eyescale/Equalizer/pull/383):
  Make discovery for resources, plugins and server library relocatable
* [382](https://github.com/Eyescale/Equalizer/pull/382):
  Make DisplayCluster a View attribute that can be set by a global/env var.
* [381](https://github.com/Eyescale/Equalizer/pull/381):
  Fix Win32 build w/ Qt, fix mouse buttons & wheel with WGL
* [380](https://github.com/Eyescale/Equalizer/pull/380):
  Fix static initialization problem w/ Qt window system

# Release 1.8 (14-Oct-2014)

* Implemented Qt window system for onscreen windows. Issue
  [21](https://github.com/Eyescale/Equalizer/issues/21) partially done.
* DisplayCluster streaming can be enabled with automatic configuration using new
  global view attributes: EQ_VIEW_SATTR_DISPLAYCLUSTER and
  EQ_VIEW_SATTR_PIXELSTREAM_NAME.
* New EqGLLibraries.cmake script for simpler OpenGL configuration in
  downstream projects
* Compression is enabled for DisplayCluster streaming
* DisplayCluster streaming is now asynchronous
