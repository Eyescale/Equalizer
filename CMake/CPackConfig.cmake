# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

#configure_file(${CMAKE_SOURCE_DIR}/CMake/Equalizer.in.spec 
#              ${CMAKE_SOURCE_DIR}/CMake/Equalizer.spec @ONLY)

set(GPUSD_PACKAGE_VERSION "" CACHE STRING "Additional build version for packages")
mark_as_advanced(GPUSD_PACKAGE_VERSION)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(CPACK_PACKAGE_NAME "gpu-sd${VERSION_MAJOR}")
else()
  set(CPACK_PACKAGE_NAME "gpu-sd")
endif()

if(APPLE)
  set(CPACK_PACKAGE_VENDOR "www.eyescale.ch") # PackageMaker doesn't like http://
else()
  set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch") # deb lintian insists on URL
endif()

set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Local and remote GPU discovery")
# set(CPACK_PACKAGE_DESCRIPTION_FILE ${gpusd_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${gpusd_SOURCE_DIR}/LICENSE.txt)
# set(CPACK_RESOURCE_FILE_README ${gpusd_SOURCE_DIR}/RELNOTES.txt)

if(GPUSD_PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION_PATCH
      ${CPACK_PACKAGE_VERSION_PATCH}-${GPUSD_PACKAGE_VERSION})
  set(CPACK_RPM_PACKAGE_RELEASE ${GPUSD_PACKAGE_VERSION})
endif()

set(CPACK_RPM_PACKAGE_LICENSE "LGPL, GPL")
# set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries/Parallel")
set(CPACK_RPM_PACKAGE_VERSION ${VERSION})

if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
  set(DPUT_HOST "ppa:eilemann/equalizer")
endif()
set(CPACK_DEBIAN_BUILD_DEPENDS libgl1-mesa-dev libx11-dev libavahi-compat-libdnssd-dev)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libgl1-mesa-glx, libavahi-compat-libdnssd1")

set(CPACK_OSX_PACKAGE_VERSION "${GPUSD_OSX_VERSION}")

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
#set(UBUNTU_LP_BUG 300472)

# components
set(CPACK_COMPONENTS_ALL dev runtime tools daemon)

set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME
  "GPU-SD libraries and applications")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION
  "Runtime components of GPU-SD: The core library, discovery modules, announcement daemon and discovery tools")

set(CPACK_COMPONENT_DEV_DISPLAY_NAME "GPU-SD development files")
set(CPACK_COMPONENT_DEV_DESCRIPTION
  "GPU-SD header and library Files for development")
set(CPACK_COMPONENT_DEV_DEPENDS runtime)

set(CPACK_COMPONENT_DAEMON_DISPLAY_NAME "GPU-SD ZeroConf daemon")
set(CPACK_COMPONENT_DAEMON_DESCRIPTION
  "GPU-SD ZeroConf annoucement daemon")
set(CPACK_COMPONENT_DAEMON_DEPENDS runtime)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "GPU-SD helper applications")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION
  "GPU-SD Helper applications")
set(CPACK_COMPONENT_TOOLS_DEPENDS runtime)

include(InstallRequiredSystemLibraries)
include(CPack)
include(UploadPPA)
if(UPLOADPPA_FOUND)
  upload_ppa(natty)
  upload_ppa(oneiric)
  add_custom_target(dput_${PROJECT_NAME} DEPENDS ${DPUT_${PROJECT_NAME}_TARGETS})
endif()
