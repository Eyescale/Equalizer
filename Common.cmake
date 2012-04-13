
# Common settings
enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_PREFIX_PATH ${SystemDrive}:/cygwin/bin)
if(CMAKE_VERSION VERSION_LESS 2.8)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

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

# Boost settings
if(MSVC)
  option(Boost_USE_STATIC_LIBS "Use boost static libs" ON)
endif()
if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS TRUE)
endif()
add_definitions(-DBOOST_ALL_NO_LIB) # Don't use 'pragma lib' on Windows

# Compiler setting
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Winvalid-pch -Wnon-virtual-dtor -Wsign-promo -Wshadow -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter -Wno-write-strings")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
  add_definitions(
    /D_CRT_SECURE_NO_WARNINGS
    /D_SCL_SECURE_NO_WARNINGS
    /wd4244 # conversion from X to Y, possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    )
endif(MSVC)

if(LINUX)
  add_definitions(-fPIC) # 64bit Linux wants this
endif(LINUX)

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
endif(APPLE)

