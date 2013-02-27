# Common settings

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
if(CMAKE_VERSION VERSION_LESS 2.8.3)
  # WAR bug
  get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/2.8.3)
endif()
if(CMAKE_VERSION VERSION_LESS 2.8.8)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/2.8.8)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/System.cmake)

enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)

if(NOT CMAKE_BUILD_TYPE)
  if(RELEASE_VERSION)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  else()
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  endif()
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
string(TOUPPER ${CMAKE_PROJECT_NAME} UPPER_PROJECT_NAME)
string(TOLOWER ${CMAKE_PROJECT_NAME} LOWER_PROJECT_NAME)
add_definitions(-D${UPPER_PROJECT_NAME}_VERSION=${VERSION})
if(NOT VERSION_ABI)
  if(RELEASE_VERSION)
    set(VERSION_ABI ${RELEASE_VERSION})
  else()
    if(VERSION_MAJOR VERSION_GREATER 0)
      set(VERSION_ABI ${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH})
    else()
      set(VERSION_ABI ${VERSION_MINOR}${VERSION_PATCH})
    endif()
  endif()
endif()
if(NOT LAST_RELEASE)
  set(LAST_RELEASE ${VERSION})
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT MSVC)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
    "${CMAKE_PROJECT_NAME} install prefix" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(OUTPUT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${OUTPUT_INCLUDE_DIR})
include_directories(BEFORE ${CMAKE_SOURCE_DIR} ${OUTPUT_INCLUDE_DIR})

if(MSVC)
  set(CMAKE_MODULE_INSTALL_PATH ${CMAKE_PROJECT_NAME}/CMake)
else()
  set(CMAKE_MODULE_INSTALL_PATH share/${CMAKE_PROJECT_NAME}/CMake)
endif()

# Boost settings
if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()
add_definitions(-DBOOST_ALL_NO_LIB) # Don't use 'pragma lib' on Windows

include(TestBigEndian)
test_big_endian(BIGENDIAN)
if(BIGENDIAN)
  add_definitions(-D${UPPER_PROJECT_NAME}_BIGENDIAN)
endif()

# Compiler settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "XL")
  set(CMAKE_COMPILER_IS_XLCXX ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_COMPILER_IS_INTEL ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_COMPILER_IS_CLANG ON)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_COMPILER_IS_GNUCXX_PURE ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  include(${CMAKE_CURRENT_LIST_DIR}/CompilerVersion.cmake)
  COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(COMMON_GCC_FLAGS "-Wall -Wextra -Winvalid-pch -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_GCC_FLAGS} ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_GCC_FLAGS} -Wnon-virtual-dtor -Wsign-promo")
  if(GCC_COMPILER_VERSION VERSION_GREATER 4.1) # < 4.2 doesn't know -isystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
  if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  endif()
elseif(CMAKE_COMPILER_IS_INTEL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-unknown-pragmas")
endif()

if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_SCL_SECURE_NO_WARNINGS
    /wd4068 # disable unknown pragma warnings
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )
    
  # By default, do not warn when built on machines using only VS Express
  # http://cmake.org/gitweb?p=cmake.git;a=commit;h=fa4a3b04d0904a2e93242c0c3dd02a357d337f77
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()

  # By default, do not warn when built on machines using only VS Express
  # http://cmake.org/gitweb?p=cmake.git;a=commit;h=fa4a3b04d0904a2e93242c0c3dd02a357d337f77
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()

  # http://www.ogre3d.org/forums/viewtopic.php?f=2&t=60015&start=0
  if(RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR")
  else()
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm500 /EHsc /GR /WX")
  endif()
elseif(EXISTS ${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.in.spec)
  configure_file(${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.in.spec
    ${CMAKE_SOURCE_DIR}/CMake/${CMAKE_PROJECT_NAME}.spec @ONLY)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)

  if(REDHAT AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    set(LIB_SUFFIX 64 CACHE STRING "Library directory suffix")
  endif()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc")
    set(LINUX_PPC 1)
  else()
    add_definitions(-fPIC)
  endif()
endif()
set(LIBRARY_DIR lib${LIB_SUFFIX})

if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH "/opt/local/") # Macports
  if(NOT CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL "")
    if(_CMAKE_OSX_MACHINE MATCHES "ppc")
      set(CMAKE_OSX_ARCHITECTURES "ppc;ppc64" CACHE
        STRING "Build architectures for OS X" FORCE)
    else()
      set(CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE
        STRING "Build architectures for OS X" FORCE)
    endif()
  endif()
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
  if (NOT CMAKE_INSTALL_NAME_DIR)
    set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  endif (NOT CMAKE_INSTALL_NAME_DIR)
  message(STATUS
    "Building ${CMAKE_PROJECT_NAME} ${VERSION} for ${CMAKE_OSX_ARCHITECTURES}")
endif(APPLE)

# hooks to gather all targets (libraries & executables)
include(CMakeParseArguments)
set(ALL_DEP_TARGETS "")
set(ALL_LIB_TARGETS "")
macro(add_executable _target)
  _add_executable(${_target} ${ARGN})
  set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
endmacro()
macro(add_library _target)
  _add_library(${_target} ${ARGN})

  # ignore IMPORTED add_library from finders (e.g. Qt)
  cmake_parse_arguments(_arg "IMPORTED" "" "" ${ARGN})
  if(NOT _arg_IMPORTED)
    # add defines TARGET_DSO_NAME and TARGET_SHARED for dlopen() usage
    get_target_property(THIS_DEFINITIONS ${_target} COMPILE_DEFINITIONS)
    if(NOT THIS_DEFINITIONS)
      set(THIS_DEFINITIONS) # clear THIS_DEFINITIONS-NOTFOUND
    endif()
    string(TOUPPER ${_target} _TARGET)

    if(MSVC OR XCODE_VERSION)
      set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      if(APPLE)
        set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}.${VERSION_ABI}${CMAKE_SHARED_LIBRARY_SUFFIX})
      else()
        set(_libraryname ${CMAKE_SHARED_LIBRARY_PREFIX}${_target}${CMAKE_SHARED_LIBRARY_SUFFIX}.${VERSION_ABI})
      endif()
    endif()

    list(APPEND THIS_DEFINITIONS
      ${_TARGET}_SHARED ${_TARGET}_DSO_NAME=\"${_libraryname}\")

    set_target_properties(${_target} PROPERTIES
      COMPILE_DEFINITIONS "${THIS_DEFINITIONS}")

    set_property(GLOBAL APPEND PROPERTY ALL_DEP_TARGETS ${_target})
    set_property(GLOBAL APPEND PROPERTY ALL_LIB_TARGETS ${_target})
  endif()
endmacro()
