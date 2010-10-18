# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Equalizer)
enable_testing()

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
  # Looking for libboost_system-vcXX-mt-gd-XXX.X.lib, 
  #  libboost_system-vcXX-mt-XXX.X.lib, libboost_regex-vcXX-mt-gd-XXX.X.lib 
  #  libboost_regex-vcXX-mt-XXX.X.lib, libboost_date_time-vcXX-mt-gd-XXX.X.lib  
  #  libboost_date_time-vcXX-mt-XXX.X.lib.
  # Use BoostPro to install compiled boost library  
  set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)
set(Boost_USE_MULTITHREADED ON)
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.43" "1.44")
find_package(Boost REQUIRED COMPONENTS system regex date_time)
add_definitions(-DBOOST_ALL_NO_LIB)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER SYSTEM ${Boost_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
  _Boost_COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(OpenMP)
if(OPENMP_FOUND)
  if(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
    message(STATUS "  Disabling OpenMP due to bug in gcc ${GCC_COMPILER_VERSION}")
  else(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
    set(OPENMP_USED 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(CMAKE_COMPILER_IS_GNUCXX AND GCC_COMPILER_VERSION LESS 43)
endif(OPENMP_FOUND)

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})

find_package(X11)
if(X11_FOUND)
  include_directories(SYSTEM ${X11_INCLUDE_DIR})
endif(X11_FOUND)


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
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/debug/)
  install(FILES ${PTHREAD_BIN_DIR}/pthreadVC2.dll
    DESTINATION bin)
else(WIN32)
  set(PTHREAD_LIBRARIES pthread)
  if(LINUX)
    add_definitions(-fPIC) # 64bit Linux wants this
  endif(LINUX)
endif(WIN32)


if(X11_FOUND AND NOT(APPLE AND OPENGL_GLU_FOUND AND EXISTS /Developer/SDKs/MacOSX10.6.sdk))
  set(GLX_USED 1)
endif(X11_FOUND AND NOT(APPLE AND OPENGL_GLU_FOUND AND EXISTS /Developer/SDKs/MacOSX10.6.sdk))


if(APPLE)
  link_directories(/usr/X11R6/lib)
  include_directories(SYSTEM /usr/X11R6/include)

  if(NOT XCODE_VERSION)
    add_definitions(-DDarwin)
  endif(NOT XCODE_VERSION)
  if(GLX_USED)
    set (CMAKE_OSX_ARCHITECTURES "x86_64")
    set (CUDA_64_BIT_DEVICE_CODE ON) 
  else(GLX_USED)
    set (CMAKE_OSX_ARCHITECTURES "i386")
    set (CUDA_64_BIT_DEVICE_CODE OFF) 
  endif(GLX_USED)
endif(APPLE)


include(${CMAKE_SOURCE_DIR}/externals/UseGLEW.cmake)
include(${CMAKE_SOURCE_DIR}/externals/UseVMMLib.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEQ_BUILD_DIR=${CMAKE_BINARY_DIR}/")

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "http://www.equalizergraphics.com/")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "2")
set(VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)

set(CPACK_COMPONENTS_ALL Unspecified dev tools data vmmlib)

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Headers and Libraries")
set(CPACK_COMPONENT_DEV_DESCRIPTION
  "Header Files and Developer Libraries")

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION
  "Some useful stuff")

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION
  "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

include(InstallRequiredSystemLibraries)
include(CPack)


add_subdirectory(admin)
add_subdirectory(examples)
add_subdirectory(lib)
add_subdirectory(server)
add_subdirectory(tests)
add_subdirectory(tools)

add_executable(eqServer server/eqServer.cpp)
target_link_libraries(eqServer EqualizerServer)
install(TARGETS eqServer DESTINATION bin)

