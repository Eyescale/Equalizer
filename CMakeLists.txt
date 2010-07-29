# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Equalizer)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CToolBox)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(EQ_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${EQ_INCLUDE_DIR})
include_directories(BEFORE ${EQ_INCLUDE_DIR})

# copy files while preserving directory structure
function(COPY_HEADERS DIR)
  foreach(FILE ${ARGN})
    get_filename_component(PATH ${FILE} PATH)
    file(COPY ${FILE} DESTINATION ${EQ_INCLUDE_DIR}/${DIR}/${PATH})
  endforeach(FILE ${ARGN})
endfunction(COPY_HEADERS DIR)

if(WIN32)
  set(Boost_USE_STATIC_LIBS   ON)
endif(WIN32)
set(Boost_USE_MULTITHREADED ON)
set(Boost_ADDITIONAL_VERSIONS "1.42" "1.43")
find_package(Boost  REQUIRED COMPONENTS system)
add_definitions(-DBOOST_ALL_NO_LIB)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(AFTER ${Boost_INCLUDE_DIRS})


find_package(OpenGL REQUIRED)
include(${CMAKE_SOURCE_DIR}/externals/UseGLEW.cmake)


find_package(OpenMP)
add_definitions(${OpenMP_CXX_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")

if(MSVC)
  add_definitions(
    /W3     # warning level
    /WX     # warnings as errors
    /D_CRT_SECURE_NO_WARNINGS
    /wd4244 # conversion from 'wide' to 'narrow', possible loss of data
    /wd4800 # forcing value to bool 'true' or 'false' (performance warning)
    /wd4018 # signed/unsigned mismatch
    /wd4065 # switch statement contains 'default' but no 'case' labels
    )
endif(MSVC)

if(WIN32)
  include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/Windows/pthreads/include)
  set(PTHREAD_LIBRARIES ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib/pthreadVC2.lib)
  file(COPY ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib/pthreadVC2.dll
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  install(FILES ${CMAKE_SOURCE_DIR}/Windows/pthreads/lib/pthreadVC2.dll
    DESTINATION bin)
else(WIN32)
  set(PTHREAD_LIBRARIES pthread)
  add_definitions(-fPIC) # 64bit Linux wants this
endif(WIN32)


add_subdirectory(admin)
add_subdirectory(examples)
#add_subdirectory(externals)
add_subdirectory(lib)
add_subdirectory(server)
add_subdirectory(tests)
add_subdirectory(tools)

add_executable(eqServer server/eqServer.cpp)
target_link_libraries(eqServer EqualizerServer)
install(TARGETS eqServer DESTINATION bin)


install(DIRECTORY ${EQ_INCLUDE_DIR}/eq DESTINATION include COMPONENT dev)

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "http://www.equalizergraphics.com/")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@equalizergraphics.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "2")

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)

set(CPACK_COMPONENTS_ALL Unspecified dev tools data)

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Headers and Libraries")
set(CPACK_COMPONENT_DEV_DESCRIPTION
  "Header Files and Developer Libraries")

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION
  "Some useful stuff")

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION
  "many config file examples and some polygon models")


include(InstallRequiredSystemLibraries)
include(CPack)
