# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Equalizer)

enable_testing()

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
  set(Boost_USE_STATIC_LIBS   ON)
endif(WIN32)
set(Boost_USE_MULTITHREADED ON)
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.43" "1.44")
find_package(Boost REQUIRED COMPONENTS system)
add_definitions(-DBOOST_ALL_NO_LIB)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER ${Boost_INCLUDE_DIRS})


find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(X11)
if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIR})
endif(X11_FOUND)

include(${CMAKE_SOURCE_DIR}/externals/UseGLEW.cmake)
include(${CMAKE_SOURCE_DIR}/externals/UseVMMLib.cmake)


find_package(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif(OPENMP_FOUND)

if(MSVC)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

if(WIN32)
  include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/Windows/pthreads/include)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PTHREAD_BIN_DIR ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib64)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PTHREAD_BIN_DIR ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

  set(PTHREAD_LIBRARIES ${PTHREAD_BIN_DIR}/pthreadVC2.lib)
  file(COPY ${PTHREAD_BIN_DIR}/pthreadVC2.dll
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  install(FILES ${PTHREAD_BIN_DIR}/pthreadVC2.dll
    DESTINATION bin)
else(WIN32)
  set(PTHREAD_LIBRARIES pthread)
  add_definitions(-fPIC) # 64bit Linux wants this
endif(WIN32)

if(APPLE)
  link_directories(/usr/X11R6/lib)
  include_directories(/usr/X11R6/include)

  if(XCODE_VERSION)
  else(XCODE_VERSION)
    add_definitions(-DDarwin)
  endif(XCODE_VERSION)
endif(APPLE)

add_subdirectory(admin)
add_subdirectory(examples)
add_subdirectory(lib)
add_subdirectory(server)
add_subdirectory(tests)
add_subdirectory(tools)

add_executable(eqServer server/eqServer.cpp)
target_link_libraries(eqServer EqualizerServer)
install(TARGETS eqServer DESTINATION bin)


#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "http://www.equalizergraphics.com/")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "2")

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
