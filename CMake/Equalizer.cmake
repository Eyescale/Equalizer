

set(EQUALIZER_PACKAGE_VERSION 1.7)
set(EQUALIZER_DEPENDS
  REQUIRED vmmlib Lunchbox Collage OpenGL Boost
  OPTIONAL X11 hwsd GLStats hwloc OpenSceneGraph OpenCV VRPN
  DisplayCluster MAGELLAN GLEW_MX Qt4)
set(EQUALIZER_DEB_DEPENDS bison flex
  libboost-program-options-dev libboost-filesystem-dev libboost-system-dev
  libx11-dev libgl1-mesa-dev libglewmx1.6-dev libspnav-dev
  libopenscenegraph-dev libopencv-dev libqt4-dev)
set(EQUALIZER_PORT_DEPENDS boost opencv)

set(EQUALIZER_BOOST_COMPONENTS "program_options filesystem system")
set(EQUALIZER_OPENSCENEGRAPH_COMPONENTS "osgDB osgUtil")
set(EQUALIZER_QT4_COMPONENTS "QtCore QtGui QtOpenGL")

set(EQUALIZER_ROOT_VAR EQ_ROOT)
set(EQUALIZER_REPO_URL https://github.com/Eyescale/Equalizer.git)
set(EQUALIZER_REPO_TAG master)
set(EQUALIZER_FORCE_BUILD ${CI_BUILD})

if(CI_BUILD_COMMIT)
  set(EQUALIZER_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(EQUALIZER_REPO_TAG master)
endif()
set(EQUALIZER_FORCE_BUILD ON)
set(EQUALIZER_SOURCE ${CMAKE_SOURCE_DIR})