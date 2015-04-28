

set(EQUALIZER_PACKAGE_VERSION 1.8)
set(EQUALIZER_REPO_URL https://github.com/Eyescale/Equalizer.git)

set(EQUALIZER_DEPENDS
  REQUIRED vmmlib Lunchbox Collage OpenGL Boost Pression
  OPTIONAL X11 hwsd GLStats hwloc OpenSceneGraph OpenCV VRPN
           Deflect MAGELLAN GLEW_MX Qt5Core Qt5OpenGL Qt5Widgets)
set(EQUALIZER_BOOST_COMPONENTS "program_options filesystem system thread")
set(EQUALIZER_OPENSCENEGRAPH_COMPONENTS "osgDB osgUtil")

set(EQUALIZER_DEB_DEPENDS bison flex libboost-program-options-dev
  libboost-filesystem-dev libboost-system-dev libboost-thread-dev
  libx11-dev libgl1-mesa-dev libglewmx1.6-dev libspnav-dev
  libopenscenegraph-dev libopencv-dev qtbase5-dev)
set(EQUALIZER_PORT_DEPENDS boost opencv)
set(EQUALIZER_ROOT_VAR EQ_ROOT)
set(EQUALIZER_SUBPROJECT ON)

if(CI_BUILD_COMMIT)
  set(EQUALIZER_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(EQUALIZER_REPO_TAG master)
endif()
set(EQUALIZER_FORCE_BUILD ON)
set(EQUALIZER_SOURCE ${CMAKE_SOURCE_DIR})