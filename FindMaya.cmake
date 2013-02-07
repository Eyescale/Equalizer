############################################################
#
# Ecole Polythechnique Federal De Lausanne (EPFL)
# Brain Mind Institute (BMI)
# Blue Brain Project (BBP)
# Copyrights (c) 2012-2013
#
# Author : Marwam Abdellah <marwan.abdellah@epfl.ch>
#
############################################################

# Checking REQUIRED or NOT
if(MAYA_FIND_REQUIRED)
  set(_maya_output_type FATAL_ERROR)
  set(_maya_output 1)
else()
  set(_maya_output_type STATUS)
  if(NOT Maya_FIND_QUIETLY)
    set(_maya_output 1)
  endif()
endif()

find_path(MAYA_ROOT_DIR include/maya/MLibrary.h
  /usr/autodesk/maya2012-x64
  /usr/autodesk/maya8.5-x64
  /usr/autodesk/maya8.0-x64
  /usr/aw/maya8.0-x64
  /usr/autodesk/maya8.5
  /usr/autodesk/maya8.0
  /usr/aw/maya8.0
  /usr/aw/maya7.0
  /usr/aw/maya6.5
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.5-x64"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.5"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.0-x64"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.0"
  "$ENV{PROGRAMFILES}/Alias/Maya8.0-x64"
  "$ENV{PROGRAMFILES}/Alias/Maya8.0"
  "$ENV{PROGRAMFILES}/Alias/Maya7.0"
  "$ENV{PROGRAMFILES}/Alias/Maya6.5"
  DOC "Root directory of Maya installation"
  )

if(MAYA_ROOT_DIR)
  # Include & libraries directories
  set(MAYA_INCLUDE_DIR "${MAYA_ROOT_DIR}/include")
  set(MAYA_LIBRARY_DIR "${MAYA_ROOT_DIR}/lib")
  set(MAYE_DEFINITIONS -D_BOOL -DFUNCPROTO -DREQUIRE_IOSTREAM)

  # Checking the libraries
  if(UNIX)
    set(MAYA_LIBRARY_NAMES OpenMayalib)
  endif()

  list(APPEND MAYA_LIBRARY_NAMES Foundation OpenMaya OpenMayaAnim OpenMayaUI
    OpenMayaRender OpenMayaFX Cloth Image)

  foreach(MAYA_LIBRARY ${MAYA_LIBRARY_NAMES})
    find_library(MAYA_${MAYA_LIBRARY}_LIBRARY ${MAYA_LIBRARY}
      ${MAYA_LIBRARY_DIR} NO_DEFAULT_PATH)
    list(APPEND MAYA_LIBRARIES ${MAYA_${MAYA_LIBRARY}_LIBRARY})
    list(APPEND MAYA_LIBRARY_CHECKS MAYA_${MAYA_LIBRARY}_LIBRARY)
  endforeach()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(MAYA DEFAULT_MSG MAYA_ROOT_DIR
    ${MAYA_LIBRARY_CHECKS})
else()
  if(${_maya_output})
    message(${_maya_output_type} "MAYA_ROOT_DIR not found")
  endif()
endif(MAYA_ROOT_DIR)

if(MAYA_FOUND AND _maya_output)
  message(STATUS "Found Maya in ${MAYA_INCLUDE_DIR}:${MAYA_LIBRARIES}")
endif()

