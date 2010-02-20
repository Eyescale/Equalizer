CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(osgScaleViewer)

# Set the module path to find FindEqualizer.cmake and FindOSG.cmake
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Look for OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
  MESSAGE(STATUS "Found OpenGL: Includes at ${OPENGL_INCLUDE_DIR}")
  MESSAGE(STATUS "Found OpenGL: Library at ${OPENGL_gl_LIBRARY}")
ELSE(OPENGL_FOUND)
  MESSAGE(FATAL_ERROR "OpenGL was NOT found!")
ENDIF(OPENGL_FOUND)

# Look for Equalizer
FIND_PACKAGE(Equalizer REQUIRED)
IF(Equalizer_FOUND)
  MESSAGE(STATUS "Found Equalizer: Includes at ${Equalizer_INCLUDE_DIR}")
  MESSAGE(STATUS "Found Equalizer: Libraries at ${Equalizer_LIBRARIES}")
ELSE(Equalizer_FOUND)
  MESSAGE(FATAL_ERROR "Equalizer was NOT found!")
ENDIF(Equalizer_FOUND)

# Look for OpenSceneGraph
FIND_PACKAGE(OSG REQUIRED)
IF(OSG_FOUND)
  MESSAGE(STATUS "Found OSG: Includes at ${OSG_INCLUDE_DIR}")
  MESSAGE(STATUS "Found OSG: Libraries at ${OSG_LIBRARIES}")
ELSE(OSG_FOUND)
  MESSAGE(FATAL_ERROR "OSG was NOT found!")
ENDIF(OSG_FOUND)

# Add OpenGL, Equalizer and OSG to the include dirctories, so that an #include
# looks into the correct directory.
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${Equalizer_INCLUDE_DIR} ${OSG_INCLUDE_DIR})

# Reduce amount of warning under MS VC++
IF(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /EHsc") 
ENDIF(WIN32)

# We default to the debug build type for now
SET(CMAKE_BUILD_TYPE Debug)

# For MS VC++: Add header files to the project tree
FILE(GLOB headers "${CMAKE_SOURCE_DIR}/*.h")

# Create the executable
ADD_EXECUTABLE(osgScaleViewer
  main.cpp
  osgScaleViewer.cpp
  channel.cpp
  config.cpp
  frameData.cpp
  initData.cpp
  node.cpp
  pipe.cpp
  sceneReader.cpp
  tracker.cpp
  window.cpp
  quad.cpp
)

# Link all needed libraries to the executable
TARGET_LINK_LIBRARIES(osgScaleViewer
  ${Equalizer_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${OSG_LIBRARIES}
)

# Install our executable
INSTALL(TARGETS osgScaleViewer
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
