# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(Equalizer)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "9")
set(VERSION_PATCH "2")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

enable_testing()

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(NOT DEFINED EQ_PREFER_AGL)
  SET(EQ_PREFER_AGL 1)
ENDIF(NOT DEFINED EQ_PREFER_AGL)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(EQ_FEATURES)

set(EQ_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${EQ_INCLUDE_DIR})
include_directories(BEFORE ${EQ_INCLUDE_DIR})

# create forwarding headers
macro(FORWARD_HEADERS DIR)
  foreach(FILE ${ARGN})
    get_filename_component(ABSOLUTE ${FILE} ABSOLUTE)
    file(RELATIVE_PATH RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ABSOLUTE})
    set(FWDFILE ${EQ_INCLUDE_DIR}/${DIR}/${RELATIVE})
    if(NOT EXISTS ${FWDFILE})
      get_filename_component(PATH ${RELATIVE} PATH)
      file(RELATIVE_PATH INCLUDE ${EQ_INCLUDE_DIR}/${DIR}/${PATH} ${ABSOLUTE})
      file(WRITE ${FWDFILE} "#include \"${INCLUDE}\"\n")
    endif(NOT EXISTS ${FWDFILE})
  endforeach(FILE ${ARGN})
endmacro(FORWARD_HEADERS DIR)


if(WIN32)
  set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)
set(Boost_USE_MULTITHREADED ON)
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.43" "1.44")
find_package(Boost REQUIRED COMPONENTS system regex date_time)
add_definitions(-DBOOST_ALL_NO_LIB)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})

find_package(OpenMP)
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX)
    _Boost_COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  endif(CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
    message(STATUS "  Disable OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
    set(EQ_OPENMP_USED 1)
    set(EQ_FEATURES "${EQ_FEATURES} OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
endif(OPENMP_FOUND)

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})

if(APPLE)
  # WAR otherwise MacPorts X11 (/opt/local) is preferred
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

find_package(X11)

if(APPLE)
  LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib)
  LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH /opt/local)
endif(APPLE)

if(X11_FOUND)
  if(APPLE)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ") # Disabled by GNU.cmake!?
    if(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
      if(EQ_PREFER_AGL)
        set(EQ_AGL_USED 1)
      else(EQ_PREFER_AGL)
        set(EQ_GLX_USED 1)
      endif(EQ_PREFER_AGL)
    else(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
      set(EQ_AGL_USED 1)
      set(EQ_GLX_USED 1)
    endif(EXISTS /Developer/SDKs/MacOSX10.6.sdk)
  else(APPLE)
    set(EQ_GLX_USED 1)
  endif(APPLE)
endif(X11_FOUND)

if(EQ_GLX_USED)
  include_directories(SYSTEM ${X11_INCLUDE_DIR})
  set(EQ_FEATURES "${EQ_FEATURES} GLX")
endif(EQ_GLX_USED)
if(EQ_AGL_USED)
  set(EQ_FEATURES "${EQ_FEATURES} AGL")
endif(EQ_AGL_USED)

find_package(CUDA)

if(MSVC)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
  add_definitions(/W2)
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings")
  if(NOT XCODE_VERSION) # XCode doesn't know -isystem...
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif(NOT XCODE_VERSION)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(WIN32)
   include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/Windows/pthreads/include)
  
   if(NOT BISON_EXECUTABLE)
     set(BISON_EXECUTABLE ${CMAKE_SOURCE_DIR}/Windows/bin/bison.exe)
   endif(NOT BISON_EXECUTABLE)
   if(NOT FLEX_EXECUTABLE)
     set(FLEX_EXECUTABLE ${CMAKE_SOURCE_DIR}/Windows/bin/flex.exe)
   endif(NOT FLEX_EXECUTABLE)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PTHREAD_BIN_DIR ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib64)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PTHREAD_BIN_DIR ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

  set(PTHREAD_LIBRARIES ${PTHREAD_BIN_DIR}/pthreadVC2.lib)
  file(COPY ${PTHREAD_BIN_DIR}/pthreadVC2.dll
       DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/debug/ )
  file(COPY ${PTHREAD_BIN_DIR}/pthreadVC2.dll
       DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/release/ )
  install(FILES ${PTHREAD_BIN_DIR}/pthreadVC2.dll DESTINATION bin)
else(WIN32)
  set(PTHREAD_LIBRARIES pthread)
  if(LINUX)
    add_definitions(-fPIC) # 64bit Linux wants this
  endif(LINUX)
endif(WIN32)


if(APPLE)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)

  if(NOT XCODE_VERSION)
    add_definitions(-DDarwin)
  endif(NOT XCODE_VERSION)
  if(EQ_AGL_USED)
    set (CMAKE_OSX_ARCHITECTURES "i386")
    set (CUDA_64_BIT_DEVICE_CODE OFF) 
  else(EQ_AGL_USED)
    set (CMAKE_OSX_ARCHITECTURES "i386;x86_64")
    set (CUDA_64_BIT_DEVICE_CODE ON) 
  endif(EQ_AGL_USED)
  if(CUDA_FOUND AND CMAKE_OSX_ARCHITECTURES MATCHES ".*64.*")
    set(CUDA_FOUND 0)
    message(STATUS "  Disable CUDA due to missing 64 bit libraries")
  endif(CUDA_FOUND AND CMAKE_OSX_ARCHITECTURES MATCHES ".*64.*")
endif(APPLE)

if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
  set(EQ_FEATURES "${EQ_FEATURES} CUDA")
endif(CUDA_FOUND)


include(${CMAKE_SOURCE_DIR}/externals/UseGLEW.cmake)
include(${CMAKE_SOURCE_DIR}/externals/UseVMMLib.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEQ_BUILD_DIR=\"${CMAKE_BINARY_DIR}/\"")


add_subdirectory(admin)
add_subdirectory(examples)
add_subdirectory(lib)
add_subdirectory(server)
add_subdirectory(tests)
add_subdirectory(tools)
    
add_executable(eqServer server/eqServer.cpp)
target_link_libraries(eqServer EqualizerServer)
install(TARGETS eqServer DESTINATION bin COMPONENT lib)
if(MSVC)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/eqServer.pdb DESTINATION bin COMPONENT dev)
endif(MSVC)


message(STATUS "Supported Features:${EQ_FEATURES}")

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL lib dev apps examples tools data vmmlib)

set(CPACK_COMPONENT_LIB_DISPLAY_NAME "Core Libraries")
set(CPACK_COMPONENT_LIB_DESCRIPTION "Equalizer Runtime Libraries")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Headers")
set(CPACK_COMPONENT_DEV_DESCRIPTION "Header Files for Developments")
set(CPACK_COMPONENT_DEV_DEPENDS vmmlib lib)

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Equalizer")
set(CPACK_COMPONENT_APPS_DEPENDS lib data)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS dev data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs around Equalizer")
set(CPACK_COMPONENT_TOOLS_DEPENDS lib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system-dev, libboost-regex-dev, libboost-date-time-dev, bison, flex, libx11-dev")

include(InstallRequiredSystemLibraries)
include(CPack)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copyScript.cmake)
