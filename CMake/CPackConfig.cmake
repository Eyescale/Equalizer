# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_VENDOR "www.eyescale.ch")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/doc/Changelog.md)
set(CPACK_RESOURCE_FILE_README ${CPACK_PACKAGE_DESCRIPTION_FILE})

set(EQ_IB_PACKAGES "librdmacm-dev, libibverbs-dev, librdmacm-dev")
set(CPACK_DEBIAN_BUILD_DEPENDS bison flex libboost-system-dev
  libboost-filesystem-dev libboost-date-time-dev libboost-regex-dev libboost-serialization-dev
  libboost-program-options-dev libx11-dev libgl1-mesa-dev libglewmx1.6-dev
  libspnav0 librdmacm-dev libibverbs-dev librdmacm-dev
  ${COLLAGE_DEB_DEV_DEPENDENCY} ${HWSD_DEB_DEV_DEPENDENCY}
  ${GLSTATS_DEB_DEV_DEPENDENCY} ${LUNCHBOX_DEB_DEV_DEPENDENCY}
  ${VMMLIB_DEB_DEPENDENCIES})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libboost-system-dev, libboost-date-time-dev, libboost-regex-dev, libboost-program-options-dev, libboost-filesystem-dev, libboost-serialization-dev, libx11-dev, libgl1-mesa-dev, libglewmx1.6-dev, libspnav-dev, ${EQ_IB_PACKAGES}, ${COLLAGE_DEB_DEV_DEPENDENCY}, ${HWSD_DEB_DEPENDENCIES}, ${GLSTATS_DEB_DEV_DEPENDENCY}, ${LUNCHBOX_DEB_DEV_DEPENDENCY}, ${VMMLIB_DEB_DEPENDENCIES}")

set(CPACK_MACPORTS_CATEGORY graphics)
set(CPACK_MACPORTS_DEPENDS boost hwsd GLStats Collage Lunchbox VMMLIB)

set(UBUNTU_LP_BUG 300472)
include(CommonCPack)
include(OSSCPack)
