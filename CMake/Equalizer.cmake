

set(EQUALIZER_PACKAGE_VERSION 1.6.1)
set(EQUALIZER_DEPENDS
  REQUIRED vmmlib Lunchbox Collage OpenGL Boost
  OPTIONAL X11 hwsd GLStats hwloc OpenSceneGraph SAGE OpenCV VRPN
  DisplayCluster MAGELLAN GLEW_MX)
set(EQUALIZER_DEB_DEPENDS bison flex libboost-program-options-dev
  libx11-dev libgl1-mesa-dev libglewmx1.6-dev libspnav-dev
  libopenscenegraph-dev libopencv-dev)

set(EQUALIZER_BOOST_COMPONENTS "program_options")
set(EQUALIZER_OPENSCENEGRAPH_COMPONENTS "osgDB osgUtil")

set(EQUALIZER_ROOT_VAR EQ_ROOT)
set(EQUALIZER_REPO_URL https://github.com/Eyescale/Equalizer.git)
set(EQUALIZER_REPO_TAG 1.6)

if(CI_BUILD_COMMIT)
  set(EQUALIZER_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(EQUALIZER_REPO_TAG master)
endif()
set(EQUALIZER_FORCE_BUILD ON)
set(EQUALIZER_SOURCE ${CMAKE_SOURCE_DIR})
