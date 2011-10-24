# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_NAME "gpusd${VERSION_ABI}")

if(APPLE)
  set(CPACK_PACKAGE_VENDOR "www.eyescale.ch") # PackageMaker doesn't like http://
else()
  set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch") # deb lintian insists on URL
endif()

set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphics Processing Unit Discovery Service")
#set(CPACK_PACKAGE_DESCRIPTION_FILE ${Gpusd_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
#set(CPACK_RESOURCE_FILE_LICENSE ${Gpusd_SOURCE_DIR}/LICENSE.txt)
#set(CPACK_RESOURCE_FILE_README ${Gpusd_SOURCE_DIR}/RELNOTES.txt)

set(CPACK_COMPONENTS_ALL lib dev daemon)

set(CPACK_COMPONENT_LIB_DISPLAY_NAME "GPU-SD Libraries")
set(CPACK_COMPONENT_LIB_DESCRIPTION "GPU-SD Runtime Libraries")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "GPU-SD Development Files")
set(CPACK_COMPONENT_DEV_DESCRIPTION "GPU-SD Development Headers")
set(CPACK_COMPONENT_DAEMON_DEPENDS lib)

set(CPACK_COMPONENT_DAEMON_DISPLAY_NAME "GPU-SD ZeroConf daemon")
set(CPACK_COMPONENT_DAEMON_DESCRIPTION "Daemon announcing GPU resources using ZeroConf")
set(CPACK_COMPONENT_DAEMON_DEPENDS lib)

set(CPACK_RPM_PACKAGE_LICENSE "LGPL, GPL")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Graphics")
set(CPACK_RPM_PACKAGE_VERSION ${VERSION})

if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
  set(DPUT_HOST "ppa:eilemann/gpusd")
endif()

set(CPACK_DEBIAN_BUILD_DEPENDS libgl1-mesa-dev)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libx11-dev, libgl1-mesa-dev, libavahi-compat-libdnssd1-dev")

SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "/sbin/ldconfig")

set(CPACK_OSX_PACKAGE_VERSION "10.5")

if(MSVC)
  set(CPACK_GENERATOR "NSIS")
endif(MSVC)

if(APPLE)
  set(CPACK_GENERATOR "PackageMaker")
  set(CPACK_SET_DESTDIR ON)
endif(APPLE)

if(LINUX)
  find_program(RPM_EXE rpmbuild)
  if(${RPM_EXE} MATCHES RPM_EXE-NOTFOUND)
    set(CPACK_GENERATOR "TGZ;DEB")
  else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
  endif()
endif(LINUX)

set(CPACK_STRIP_FILES TRUE)
include(InstallRequiredSystemLibraries)
include(CPack)
#include(UploadPPA)
