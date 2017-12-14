# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>
#               2017 Raphael Dumusc <raphael.dumusc@epfl.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_VENDOR "Eyescale")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/doc/Changelog.md)
set(CPACK_RESOURCE_FILE_README ${CPACK_PACKAGE_DESCRIPTION_FILE})

set(EQUALIZER_PACKAGE_DEB_DEPENDS ${EQUALIZER_DEB_DEPENDS})
list(REMOVE_ITEM EQUALIZER_PACKAGE_DEB_DEPENDS bison flex)
add_deb_depends(Collage libstdc++6 libboost-system-dev libboost-date-time-dev
  libboost-regex-dev libboost-serialization-dev libboost-thread-dev
  librdmacm-dev libibverbs-dev librdmacm-dev libudt-dev)
add_deb_depends(Deflect PACKAGE_NAME desktopstreamer qtbase5-dev libturbojpeg)
add_deb_depends(GLStats libstdc++6 libgl1-mesa-glx)
add_deb_depends(hwsd libstdc++6 libgl1-mesa-glx libboost-program-options-dev
  libboost-regex-dev libboost-system-dev qtbase5-dev)
add_deb_depends(Lunchbox libboost-filesystem-dev libboost-regex-dev
  libboost-serialization-dev libboost-system-dev libhwloc-dev)
add_deb_depends(Pression libboost-program-options-dev)
add_deb_depends(Servus libavahi-client-dev avahi-daemon)
add_deb_depends(vmmlib libstdc++6)

set(CPACK_DEBIAN_BUILD_DEPENDS ${EQUALIZER_DEB_DEPENDS})

set(CPACK_MACPORTS_CATEGORY graphics)
set(CPACK_MACPORTS_DEPENDS boost hwsd GLStats Collage Lunchbox VMMLIB)

set(UBUNTU_LP_BUG 300472)
include(CommonCPack)
include(OSSCPack)
