# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "Eyescale")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${Equalizer_SOURCE_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README ${Equalizer_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL lib dev man apps examples tools data vmmlib)

set(CPACK_COMPONENT_LIB_DISPLAY_NAME "Core Libraries")
set(CPACK_COMPONENT_LIB_DESCRIPTION "Equalizer and Collage Runtime Libraries")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "Headers")
set(CPACK_COMPONENT_DEV_DESCRIPTION "Header Files for Development")
set(CPACK_COMPONENT_DEV_DEPENDS vmmlib lib)

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Man Pages")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Manual Pages")
set(CPACK_COMPONENT_MAN_DEPENDS dev)

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Equalizer")
set(CPACK_COMPONENT_APPS_DEPENDS lib data)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS dev data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs for Equalizer")
set(CPACK_COMPONENT_TOOLS_DEPENDS lib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(CPACK_COMPONENT_FASTJPEG_DISPLAY_NAME "FastJpeg Plugin")
set(CPACK_COMPONENT_FASTJPEG_DESCRIPTION "High-Performance Compression Plugin")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system-dev, libboost-regex-dev, libboost-date-time-dev, bison, flex, libx11-dev")

if(MSVC)
  set(CPACK_GENERATOR NSIS)
  set(CPACK_NSIS_PACKAGE_NAME "Equalizer")
  set(CPACK_NSIS_DISPLAY_NAME "Equalizer Parallel Rendering Framework")
endif(MSVC)

include(InstallRequiredSystemLibraries)
include(CPack)
