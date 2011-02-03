# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "Equalizer")
set(CPACK_PACKAGE_VENDOR "Eyescale")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${Equalizer_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${Equalizer_SOURCE_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README ${Equalizer_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL colib codev eqlib eqdev man doc apps examples tools data vmmlib)

set(CPACK_COMPONENT_COLIB_DISPLAY_NAME "Collage Library")
set(CPACK_COMPONENT_COLIB_DESCRIPTION "Collage Runtime Library")

set(CPACK_COMPONENT_CODEV_DISPLAY_NAME "Collage Development Files")
set(CPACK_COMPONENT_CODEV_DESCRIPTION "Header and Library Files for Collage Development")
set(CPACK_COMPONENT_CODEV_DEPENDS colib)

set(CPACK_COMPONENT_EQLIB_DISPLAY_NAME "Equalizer Libraries")
set(CPACK_COMPONENT_EQLIB_DESCRIPTION "Equalizer Runtime Libraries")
set(CPACK_COMPONENT_EQLIB_DEPENDS colib)

set(CPACK_COMPONENT_EQDEV_DISPLAY_NAME "Equalizer Development Files")
set(CPACK_COMPONENT_EQDEV_DESCRIPTION "Header and Library Files for Equalizer Development")
set(CPACK_COMPONENT_EQDEV_DEPENDS vmmlib eqlib codev)

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Man Pages")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Manual Pages")

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Auxiliary Documentation: README, License, etc.")

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Equalizer")
set(CPACK_COMPONENT_APPS_DEPENDS eqlib data)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS eqdev data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs for Equalizer")
set(CPACK_COMPONENT_TOOLS_DEPENDS eqlib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(CPACK_RPM_PACKAGE_LICENSE "LGPL, BSD")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries/Parallel")
set(CPACK_RPM_PACKAGE_VERSION ${SHORT_VERSION})

if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Stefan Eilemann <eile@eyescale.ch>")
endif()
set(CPACK_DEBIAN_PACKAGE_SECTION "Development")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system-dev, libboost-regex-dev, libboost-date-time-dev, bison, flex, libx11-dev")

set(CPACK_OSX_PACKAGE_VERSION "10.5")

if(EQ_REVISION)
  set(CPACK_RPM_PACKAGE_RELEASE ${EQ_REVISION})
endif()
if(MSVC)
  set(CPACK_GENERATOR NSIS)
endif(MSVC)
if(APPLE)
  set(CPACK_GENERATOR PackageMaker)
endif(APPLE)

include(InstallRequiredSystemLibraries)
include(CPack)
