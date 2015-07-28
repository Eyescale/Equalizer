
This file lists all changes in the public Equalizer API, latest on top:

-----------------------------------2.0-------------------------------------
[18c2fa8] Fixed error reporting
* Config::update() reports false on failing resource, independently of
  robustness
* New function Config::getErrors() to get last errors
* Error class is serializable and contains optional originator ID
* Config::sendError() signature changed, originator is part of Error

[3458316] Fix #326: Remove Channel::getInput/OutputFrames
* The input and output frames are now passed to frameAssemble and
  frameReadback, respectively.

[b2a92d4] Close #69: WindowSettings to decouple Window from SystemWindow
* eq::fabric::Window::IATTR* are now eq::fabric::WindowSettings::IATTR*
* eq::SystemWindow:
** need eq::NotifierInterface and eq::WindowSettings for construction
** remove getWindow(), getPipe(), getNode() and getConfig()
** add setter and getter for PixelViewport, name and sharedContextWindow
* remove non-const getSharedContextWindow() from eq::Window
* eq::WindowSystemIF::createWindow() needs eq::WindowSettings

[aa93fa8] Removed eqConfigTool which was superseeded by autoconfig

[c36ea6b] Close #237: Replace setError with error events
* replace setError/getError with sendEvent and default
  Config::handleEvent implementation
* FrameBufferObject, PixelBufferObject:
** remove setError/getError
** Incompatible return value change from bool to Error for FBO::init,
   FBO::resize, PBO::setup
* Internal error events have an arbitrary number of std::string appended
* add handleEvents() in Config::init and Config::update

[7890fa5] Remove deprecated functions:
* eq::ConfigParams type. Use eq::fabric::ConfigParams.
* eq::Frame::getData API. Use Frame::getFrameData.
* WindowSystemEnum. Use WindowSystem( name ).
* eq::COMMANDTYPE_EQ_CUSTOM. Use co::COMMANDTYPE_CUSTOM.
* eq::fabric::Serializable type. Use co::Serializable.

[007e813] Remove deprecated window system types. Use their current
counterpart in the eq::agl, eq::glx and eq::wgl namespaces. Moved
XGetCurrentDisplay() and SetCurrentDisplay() from the eq to the
eq::glx namespace.

[Collage 5bb65c6] Remove most legacy EQ_ defines. Use lunchbox
namespace for co::base and co::Global.

[Collage 6f82b69] Remove legacy EQ_ macros: EQ_STDEXT_NAMESPACE_OPEN
EQ_STDEXT_NAMESPACE_CLOSE EQ_MIN EQ_MAX EQ_MAX_UINT32 EQ_BIT1 EQ_BIT2
EQ_BIT3 EQ_BIT4 EQ_BIT5 EQ_BIT6 EQ_BIT7 EQ_BIT8 EQ_BIT9 EQ_BIT10
EQ_BIT11 EQ_BIT12 EQ_BIT13 EQ_BIT14 EQ_BIT15 EQ_BIT16 EQ_BIT17
EQ_BIT18 EQ_BIT19 EQ_BIT20 EQ_BIT21 EQ_BIT22 EQ_BIT23 EQ_BIT24
EQ_BIT25 EQ_BIT26 EQ_BIT27 EQ_BIT28 EQ_BIT29 EQ_BIT30 EQ_BIT31
EQ_BIT32 EQ_BIT33 EQ_BIT34 EQ_BIT35 EQ_BIT36 EQ_BIT37 EQ_BIT38
EQ_BIT39 EQ_BIT40 EQ_BIT41 EQ_BIT42 EQ_BIT43 EQ_BIT44 EQ_BIT45
EQ_BIT46 EQ_BIT47 EQ_BIT48 EQ_BIT_ALL_32 EQ_BIT_ALL_64 EQ_BIT_NONE
EQ_TS_VAR EQ_TS_SCOPED EQ_TS_RESET EQ_TS_THREAD EQ_TS_NOT_THREAD
EQ_ALIGN8 EQ_ALIGN16 EQ_TIMEOUT_INDEFINITE EQ_TIMEOUT_DEFAULT
EQ_DLLIMPORT EQ_DLLEXPORT EQ_UNDEFINED_UINT32 EQ_GCC_4_0_OR_LATER
EQ_GCC_4_1_OR_LATER EQ_GCC_4_2_OR_LATER EQ_GCC_4_3_OR_LATER
EQ_GCC_4_4_OR_LATER EQ_GCC_4_5_OR_LATER EQ_GCC_4_6_OR_LATER
EQ_GCC_4_7_OR_LATER EQ_GCC_4_8_OR_LATER EQ_GCC_4_9_OR_LATER EQ_1KB
EQ_10KB EQ_100KB EQ_1MB EQ_10MB EQ_100MB EQ_16KB EQ_32KB EQ_64KB
EQ_128KB EQ_48MB EQ_64MB EQ_ASSERT EQ_ASSERTINFO EQ_ERROR EQ_WARN
EQ_INFO EQ_VERB EQ_CHECK EQ_UNIMPLEMENTED EQ_UNREACHABLE EQ_DONTCALL
EQ_ABORT EQ_LOG EQ_SAFECAST: Replace EQ_ with LB_

[6ee6727] Simplify ObjectManager and BitmapFont APIs:
* Replace template key type by 'void *' which was used exclusively
* Use ObjectManager references instead of pointers throughout API
* Needed for #83

To port, replace:
* eq::Window::ObjectManager* -> eq::util::ObjectManager&
* eq::Window::Font* -> eq::util::BitmapFont*

-----------------------------------1.6-------------------------------------

03/Dec/2012
  Remove ...,     fabric::View::getEqualizer()(::getTileSize( ))

07/Sep/2012
  Removed COMMANDTYPE_EQ_CUSTOM. Use co::COMMANDTYPE_CUSTOM instead.

06/Sep/2012
  New API in eq::Config & seq::ViewData for event handling. sendEvent() &
  handleEvent() have changed signatures, getNextEvent() supersedes
  tryNextEvent() & nextEvent().

  eq::ConfigEvent is replaced by EventOCommand for sending and by
  eq::EventCommand for receiving events.
  Old API is deprecated and will be removed in version 2.0.

31/Jul/2012
  Moved ErrorRegistry from Collage to eq::fabric. Use
  eq::fabric::Global::getErrorRegistry().

  Removed co::DataIStream::advanceBuffer. New parameter for
  co::DataIStream::getRemainingBuffer() does advance buffer now.

27/Jul/2012
  Made co::DataIStream::read private. Use 'is >> co::Array< T >( ptr, num )'
  instead.

-----------------------------------1.4-------------------------------------

-----------------------------------1.2-------------------------------------

25/Oct/2011
  Removed co::Object::commitNB and commitSync since request is no longer
  dispatched to command thread. Use commit instead.

17/Jul/2011
  Moved installed client headers to eq/client. Applications should
  always use <eq/eq.h> instead of individual headers.

03/Jun/2011
  Added a return value and timeout to co::LocalNode::acquireSendToken(),
  see method documentation.

29/Mar/2011
  Changed
    uint32_t eq::Version::getRevision();
  to:
    std::string eq::Version::getRevision();


-----------------------------------1.0-------------------------------------

14/Mar/2011
  Changed object mapping behavior when using a concrete version from:
    If the requested version does no longer exist, mapObject() will fail.
  to:
    If a concrete requested version no longer exists, mapObject() will
    map the oldest available version.

  Moved include/GL to include/eq/GL. Use '#include <eq/gl.h>' to include
  OpenGL headers only.

10/Mar/2011
  Do not include window system headers by default to avoid global
  namespace pollution. Define EQ_SYSTEM_INCLUDES before including
  eq/eq.h to get previous definitions.

01/Mar/2011
  Added new 'incarnation' parameter to co::Object::commit, with a
  default parameter selecting the previous behavior for commit wrt auto
  obsoletion. co::Object::commitNB has the same new parameter, with no
  default value.

--------------------------------1.0 beta-----------------------------------

10/Feb/2011
  Changed GLXWindow implementation to use GLXFBConfigs. Use GLXFBConfigs
  in place of XVisualInfo for the appropriate GLXWindow methods.

14/Jan/2011
  libEqualizer links libCollage dynamically, not statically.
  All libraries use correct versioning as mandated by the operating system.

-------------------------------1.0 alpha-----------------------------------

7/Jan/2011
  Rename EVENT_POINTER_* to differantiate channel and window pointer events.

     CHANNEL_POINTER_MOTION         // = POINTER_MOTION
     CHANNEL_POINTER_BUTTON_PRESS   // = POINTER_BUTTON_PRESS
     CHANNEL_POINTER_BUTTON_RELEASE // = POINTER_BUTTON_RELEASE
     WINDOW_POINTER_WHEEL           // = POINTER_WHEEL
     WINDOW_POINTER_MOTION
     WINDOW_POINTER_BUTTON_PRESS
     WINDOW_POINTER_BUTTON_RELEASE
  replaces
     POINTER_MOTION
     POINTER_BUTTON_PRESS
     POINTER_BUTTON_RELEASE
     POINTER_WHEEL

20/Dec/2010
  Renamed the eq::net namespace to co(llage). Change all prefixes from
  eq::net to co.

14/Dec/2010
  Removed net::Session. Use net::LocalNode or eq::Config of object
  registration and mapping. The latter retains buffered object data for
  mapping up to latency frames.

29/Nov/2010
  No automatic listener connections are created for the application
  node. Existing multi-node configs will fail if they do not configure a
  listener. AppNode listeners can be added either by specifying them in
  the configuration filem by using the --eq-listen command line option
  or programmatically by adding connection descriptions before
  Client::initLocal().

  Image::setAlphaUsage
  Image::getAlphaUsage
  Image::upload( buffer, texture, position, objectManager )
    replaces
  Image::enableAlphaUsage
  Image::disableAlphaUsage
  Image::ignoreAlpha
  Image::upload( buffer, position, objectManager )
  Image::upload( buffer, texture, objectManager )

  Image::hasData is removed


22/Nov/2010
  Replace object identifiers with UUIDs
    Replace defines EQ_ID_INVALID with the new type by UUID::ZERO for object identifiers
    Removed defines EQ_ID_MAX, NONE, INVALID, ANY for object identifiers
  Replace object versions with uint128_t
    Master versions have always 0 for the 64bit high value

  change frameID type to uint128_t for methods: frameReadback, frameAssemble,
   frameDraw, frameClear, frameStart, startFrame, frameViewStart,
   frameViewFinish, frameFinish, frameDrawFinish, frameTasksFinish
   in the classes window, channel, pipe, node and config.

  change initID type to uint128_t for configInit in pipe, window,
  channel and config

  bool Pipe::configInitSystemPipe( const uint128_t& initID );
  bool Window::configInitSystemWindow( const eq::uint128_t& initID );
  bool Window::configInitGL( const eq::uint128_t& initID );
  bool Session::mapObject( Object* object, const base::UUID& id,
                           const uint128_t& version = VERSION_OLDEST );
  uint32_t Session::mapObjectNB( Object* object, const base::UUID& id,
                                 const uint128_t& version = VERSION_OLDEST );
  void Object::notifyNewHeadVersion( const uint128_t& version );
  uint128_t Object::getOldestVersion() const;
  uint128_t Object::getVersion() const;
  uint128_t Object::getHeadVersion() const;
  uint128_t Object::sync( const uint128_t& version = VERSION_HEAD );
  uint128_t Object::commit();
  uint128_t Object::commitSync( const uint32_t commitID );
  base::UUID Object::getID() const
    replaces
  bool Pipe::configInitSystemPipe( const uint32_t initID );
  bool Window::configInitSystemWindow( const uint32_t initID );
  bool Window::configInitGL( const uint32_t initID );
  bool Session::mapObject( Object* object, const uint32_t id,
                           const uint128_t& version = VERSION_OLDEST );
  uint32_t Session::mapObjectNB( Object* object, const uint32_t id,
                                 const uint128_t& version = VERSION_OLDEST );
  void Object::notifyNewHeadVersion( const uint32_t version );
  uint32_t Object::getOldestVersion() const;
  uint32_t Object::getVersion() const;
  uint32_t Object::getHeadVersion() const;
  uint32_t Object::sync( const uint32_t version = VERSION_HEAD );
  uint32_t Object::commit();
  uint32_t Object::commitSync( const uint32_t commitID );
  uint32_t Object::getID() const

19/Nov/2010
  The GLXEventHandler is new per-window. Event handler init/exit has
  been moved from GLXPipe to GLXWindow and the GLXEventHandler API has
  changed accordingly.

15/Nov/2010
  Statistic::CHANNEL_FRAME_WAIT_READY replaces Statistic::CHANNEL_WAIT_FRAME

02/Nov/2010
  void fabric::Object::setError( const int32_t error )
  base::Error fabric::Object::getError() const
    replace
  void fabric::Object::setErrorMessage( const std::string& error )
  const std::string& fabric::Object::getErrorMessage() const

29/Oct/2010
  void FrameData::setZoom( const Zoom& zoom );
  const Zoom& FrameData::getZoom() const;
    replaces
  void Frame::setInputZoom( const Zoom& zoom );
  const Zoom& Frame::getInputZoom() const;


25/Oct/2010
  Error util::FrameBufferObject::getError() const;
  Error ComputeContext::getError() const;
  void ComputeContext::setError( const uint32_t error );
    replaces
  const std::string& util::FrameBufferObject::getErrorMessage() const;
  const std::string& ComputeContext::getErrorMessage() const;
  void ComputeContext::setErrorMessage( const std::string& errorMessage );

20/Oct/2010
  void PluginRegistry::addDirectory( const std::string& path );
  void PluginRegistry::removeDirectory( const std::string& path );
  const Strings& PluginRegistry::getDirectories() const;
    replaces
  const Strings& Global::getPluginDirectories();
  static void Global::addPluginDirectory( const std::string&
  static void Global::removePluginDirectory( const std::string& path );


19/Oct/2010
  eq::net::Node::disconnect( NodePtr ) replaces ::close( NodePtr )

18/Oct/2010
  eq::PixelData replaces eq::Image::PixelData
    eq::Image method signatures change accordingly

07/Oct/2010
  removed eq::net::Object::makeThreadSafe and isThreadSafe
    application has to lock sync(), if needed
  eq::net::Object::getAutoObsolete replaces getAutoObsoleteCount

06/Oct/2010
  void eq::Client::clientLoop
  void eq::Client::exitClient
    replaces
  bool eq::net::Node::clientLoop
  bool eq::net::Node::exitClient
