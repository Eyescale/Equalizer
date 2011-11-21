# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

#info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

#configure_file(${CMAKE_SOURCE_DIR}/CMake/Equalizer.in.spec 
#              ${CMAKE_SOURCE_DIR}/CMake/Equalizer.spec @ONLY)

set(GPUSD_PACKAGE_VERSION "" CACHE STRING "Additional build version for packages")
mark_as_advanced(GPUSD_PACKAGE_VERSION)

set(CPACK_PACKAGE_NAME "gpu-sd${VERSION_ABI}")

if(APPLE)
  set(CPACK_PACKAGE_VENDOR "www.eyescale.ch") # PackageMaker doesn't like http://
else()
  set(CPACK_PACKAGE_VENDOR "http://www.eyescale.ch") # deb lintian insists on URL
endif()

set(CPACK_PACKAGE_CONTACT "Stefan Eilemann <eile@eyescale.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Parallel Rendering Framework")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${gpusd_SOURCE_DIR}/RELNOTES.txt)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${gpusd_SOURCE_DIR}/LICENSE.txt)
set(CPACK_RESOURCE_FILE_README ${gpusd_SOURCE_DIR}/RELNOTES.txt)

if(EQ_REVISION AND NOT GPUSD_RELEASE)
  set(CPACK_PACKAGE_VERSION_PATCH ${CPACK_PACKAGE_VERSION_PATCH}.${EQ_REVISION})
endif()
if(GPUSD_NIGHTLY)
  set(CPACK_PACKAGE_VERSION_PATCH
      ${CPACK_PACKAGE_VERSION_PATCH}-${GPUSD_BUILD_ARCH})
endif()
if(GPUSD_PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION_PATCH
      ${CPACK_PACKAGE_VERSION_PATCH}-${GPUSD_PACKAGE_VERSION})
endif()

set(CPACK_COMPONENTS_ALL dev runtime)

set(CPACK_COMPONENT_COLIB_DISPLAY_NAME "Collage Library")
set(CPACK_COMPONENT_COLIB_DESCRIPTION "Collage Runtime Library")

set(CPACK_COMPONENT_CODEV_DISPLAY_NAME "Collage Development Files")
set(CPACK_COMPONENT_CODEV_DESCRIPTION "Header and Library Files for Collage Development")
set(CPACK_COMPONENT_CODEV_DEPENDS colib)

set(CPACK_COMPONENT_EQLIB_DISPLAY_NAME "Gpusd Libraries")
set(CPACK_COMPONENT_EQLIB_DESCRIPTION "Gpusd Runtime Libraries")
set(CPACK_COMPONENT_EQLIB_DEPENDS colib)

set(CPACK_COMPONENT_EQDEV_DISPLAY_NAME "Gpusd Development Files")
set(CPACK_COMPONENT_EQDEV_DESCRIPTION "Header and Library Files for Gpusd Development")
set(CPACK_COMPONENT_EQDEV_DEPENDS vmmlib eqlib codev)

set(CPACK_COMPONENT_MAN_DISPLAY_NAME "Man Pages")
set(CPACK_COMPONENT_MAN_DESCRIPTION "Manual Pages")

set(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOC_DESCRIPTION "Auxiliary Documentation: README, License, etc.")

set(CPACK_COMPONENT_APPS_DISPLAY_NAME "Example Applications")
set(CPACK_COMPONENT_APPS_DESCRIPTION "Example programs build with Gpusd")
set(CPACK_COMPONENT_APPS_DEPENDS eqlib data)

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples Source Code")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Source code of example programs")
set(CPACK_COMPONENT_EXAMPLES_DEPENDS eqdev data)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Utility programs for Gpusd")
set(CPACK_COMPONENT_TOOLS_DEPENDS eqlib)

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example configuration files and data sets")

set(CPACK_COMPONENT_VMMLIB_DISPLAY_NAME "VMMLib header files")
set(CPACK_COMPONENT_VMMLIB_DESCRIPTION
  "vmmlib is a vector and matrix math library implemented using C++ templates, thus making it very easy to integrate into other libraries and programs.")

set(CPACK_RPM_PACKAGE_LICENSE "LGPL, BSD")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries/Parallel")
set(CPACK_RPM_PACKAGE_VERSION ${VERSION})

if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
  if(GPUSD_RELEASE)
    set(DPUT_HOST "ppa:eilemann/gpusd")
  else()
    set(DPUT_HOST "ppa:eilemann/gpusd-dev")
  endif()
endif()
set(CPACK_DEBIAN_BUILD_DEPENDS bison flex libgl1-mesa-dev)

set(EQ_IB_PACKAGES "librdmacm-dev, libibverbs-dev, librdmacm-dev")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libboost-system-dev, libx11-dev, libgl1-mesa-dev, libglewmx1.5-dev, ${EQ_IB_PACKAGES}")

SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "/sbin/ldconfig")

set(CPACK_OSX_PACKAGE_VERSION "10.5")

if(EQ_REVISION)
  set(CPACK_RPM_PACKAGE_RELEASE ${EQ_REVISION})
endif()

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
set(UBUNTU_LP_BUG 300472)
include(InstallRequiredSystemLibraries)
include(CPack)
#include(UploadPPA)
