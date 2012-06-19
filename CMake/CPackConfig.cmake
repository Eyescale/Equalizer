# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

configure_file(${CMAKE_SOURCE_DIR}/CMake/Equalizer.in.spec 
  ${CMAKE_SOURCE_DIR}/CMake/Equalizer.spec @ONLY)

set(CPACK_PACKAGE_VENDOR "www.eyescale.ch")
set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${Equalizer_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_RESOURCE_FILE_README ${Equalizer_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL colib codev eqlib eqdev seqlib seqdev man doc apps examples tools data vmmlib)

set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "Misc")
set(CPACK_COMPONENT_UNSPECIFIED_DESCRIPTION "Miscellanous")

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

set(CPACK_COMPONENT_SEQLIB_DISPLAY_NAME "Sequel Libraries")
set(CPACK_COMPONENT_SEQLIB_DESCRIPTION "Sequel Runtime Libraries, a simple interface for Equalizer")
set(CPACK_COMPONENT_SEQLIB_DEPENDS eqlib colib)

set(CPACK_COMPONENT_SEQDEV_DISPLAY_NAME "Sequel Development Files")
set(CPACK_COMPONENT_SEQDEV_DESCRIPTION "Header and Library Files for Sequel Development, a simple interface for Equalizer")
set(CPACK_COMPONENT_SEQDEV_DEPENDS eqdev codev)

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Man Pages")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Manual Pages")

set(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOC_DESCRIPTION "Auxiliary Documentation: README, License, etc.")

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Equalizer")
set(CPACK_COMPONENT_APPS_DEPENDS seqlib data eqlib colib)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS seqdev eqdev codev vmmlib data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs for Equalizer")
set(CPACK_COMPONENT_TOOLS_DEPENDS eqlib colib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(EQ_IB_PACKAGES "librdmacm-dev, libibverbs-dev, librdmacm-dev")
set(CPACK_DEBIAN_BUILD_DEPENDS bison flex libboost-system-dev
  libboost-date-time-dev libboost-regex-dev libboost-serialization-dev
  libx11-dev libgl1-mesa-dev libglewmx1.5-dev
  librdmacm-dev libibverbs-dev librdmacm-dev
  ${GPUSD_DEB_BUILD_DEPENDENCIES} ${LUNCHBOX_DEB_BUILD_DEPENDENCIES}
  ${VMMLIB_DEB_BUILD_DEPENDENCIES})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libboost-system-dev, libboost-date-time-dev, libboost-regex-dev, libboost-serialization-dev, libx11-dev, libgl1-mesa-dev, libglewmx1.5-dev, ${EQ_IB_PACKAGES}, ${GPUSD_DEB_DEPENDENCIES}, ${LUNCHBOX_DEB_BUILD_DEPENDENCIES} ${VMMLIB_DEB_BUILD_DEPENDENCIES}")

set(UBUNTU_LP_BUG 300472)
include(CommonCPack)
