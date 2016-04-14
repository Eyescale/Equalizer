
# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  list(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib /usr/local/lib)
  list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local /usr/local)
  link_directories(/opt/X11/lib /usr/X11R6/lib)
  include_directories(SYSTEM /opt/X11/include /usr/X11R6/include)
endif()

common_find_package(Boost REQUIRED COMPONENTS program_options filesystem system thread)
common_find_package(Collage REQUIRED)
# broken: common_find_package(CUDA)
common_find_package(Deflect)
common_find_package(GLStats)
common_find_package(hwloc)
common_find_package(hwsd)
common_find_package(Lunchbox REQUIRED)
common_find_package(MAGELLAN)
common_find_package(OpenCV)
common_find_package(OpenGL REQUIRED)
common_find_package(OpenSceneGraph SYSTEM COMPONENTS osgDB osgUtil)
common_find_package(Pression REQUIRED)
common_find_package(Qt5Core)
common_find_package(Qt5Gui)
common_find_package(Qt5Widgets)
common_find_package(vmmlib REQUIRED)
common_find_package(VRPN)
common_find_package(X11 SYSTEM)
if(APPLE) # tweak window systems before finding glew
  if(EQ_AGL_USED)
    common_find_package_disable(X11)
  else()
    if(EQUALIZER_PREFER_QT AND Qt5Gui_FOUND)
      common_find_package_disable(X11)
    elseif(X11_FOUND)
      common_find_package_disable(Qt5Core Qt5Gui Qt5Widgets)
    endif()
  endif()
  if(CMAKE_OSX_ARCHITECTURES MATCHES "64")
    common_find_package_disable(CUDA)
  endif()
  set(CUDA_64_BIT_DEVICE_CODE OFF)
endif()
common_find_package(GLEW_MX)

list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_MX GLEW_NO_GLU) # always define GLEW_MX
if(GLEW_MX_FOUND)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_FOUND_GLEW_MX)
else()
  list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_BUILD EQ_GLEW_INTERNAL)
  if(X11_FOUND AND APPLE)
    list(APPEND COMMON_FIND_PACKAGE_DEFINES GLEW_APPLE_GLX)
  endif()
endif()

if(WIN32)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES WGL) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_WGL_USED)
endif()

if(Qt5Gui_FOUND AND Qt5Gui_VERSION VERSION_GREATER 5.0)
  set(EQ_QT_USED 1)
endif()

if(X11_FOUND)
  set(EQ_GLX_USED 1)
endif()

include(EqGLLibraries)

if(EQ_GLX_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES GLX) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_GLX_USED)
  if(MAGELLAN_FOUND AND NOT APPLE)
    list(APPEND COMMON_FIND_PACKAGE_DEFINES EQUALIZER_USE_MAGELLAN_GLX)
  endif()
elseif(EQ_AGL_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES AGL) # deprecated
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_AGL_USED)
endif()

if(EQ_QT_USED)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQ_QT_USED)
endif()

if(NOT EQUALIZER_USE_OSG)
  set(OPENSCENEGRAPH_FOUND)
endif()

if(NOT EQUALIZER_USE_HWLOC)
  common_find_package_disable(hwloc)
  set(HWLOC_GL_FOUND)
endif()
if(HWLOC_GL_FOUND)
  list(APPEND COMMON_FIND_PACKAGE_DEFINES EQUALIZER_USE_HWLOC_GL)
endif()

common_find_package_post()

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib /usr/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local /usr/local)
endif()

if(CUDA_FOUND)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
endif()
