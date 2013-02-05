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

FIND_PATH(MAYA_ROOT_DIR include/maya/MLibrary.h
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
  DOC "Root directory of Maya"
 )

IF(MAYA_ROOT_DIR)
  IF(${_maya_output})
        MESSAGE(STATUS "MAYA_ROOT_DIR is found in : ${MAYA_ROOT_DIR}")
  ENDIF()

  # Include & libraries directories
  SET(MAYA_INCLUDE_DIR "${MAYA_ROOT_DIR}/include")
  SET(MAYA_LIBRARY_DIR "${MAYA_ROOT_DIR}/lib")

  # Checking the libraries
  IF(UNIX)
      SET(MAYA_APP_LIBRARIES OpenMayalib)
  ELSE(UNIX)
      SET(MAYA_APP_LIBRARIES "")
  ENDIF(UNIX)

  # Foundation library
  FIND_LIBRARY(MAYA_Foundation_LIBRARY Foundation
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # OpenMaya library
  FIND_LIBRARY(MAYA_OpenMaya_LIBRARY OpenMaya
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # Maya animation library
  FIND_LIBRARY(MAYA_OpenMayaAnim_LIBRARY OpenMayaAnim
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # OpenMayaUI library
  FIND_LIBRARY(MAYA_OpenMayaUI_LIBRARY OpenMayaUI
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # OpenMayaRender library
  FIND_LIBRARY(MAYA_OpenMayaRender_LIBRARY OpenMayaRender
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # OpenMayaFX library
  FIND_LIBRARY(MAYA_OpenMayaFX_LIBRARY OpenMayaFX
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # Maya cloth library
  FIND_LIBRARY(MAYA_Cloth_LIBRARY Cloth
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # Maya image library
  FIND_LIBRARY(MAYA_Image_LIBRARY Image
      ${MAYA_LIBRARY_DIR}
      NO_DEFAULT_PATH
   )

  # Adding libraries to the MAYA_LIBRARY list to be linking against
  SET(MAYA_LIBRARIES ${MAYA_Foundation_LIBRARY}
                     ${MAYA_OpenMaya_LIBRARY}
                     ${MAYA_OpenMayaAnim_LIBRARY}
                     ${MAYA_OpenMayaUI_LIBRARY} 
                     ${MAYA_OpenMayaRender_LIBRARY}
                     ${MAYA_OpenMayaFX_LIBRARY}
                     ${MAYA_Cloth_LIBRARY}
                     ${MAYA_Image_LIBRARY})

  # Adding BASIC definitions
  ADD_DEFINITIONS(-D_BOOL -DFUNCPROTO -DREQUIRE_IOSTREAM)

  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(MAYA DEFAULT_MSG MAYA_ROOT_DIR 
	MAYA_LIBRARIES)

ELSE(MAYA_ROOT_DIR)
  IF(${_maya_output})
    MESSAGE(${_maya_output_type} "MAYA_ROOT_DIR is NOT found")
  ENDIF()
ENDIF(MAYA_ROOT_DIR)

IF(MAYA_FOUND AND _maya_output)
  MESSAGE(STATUS "Found MAYA in ${MAYA_INCLUDE_DIR} ${MAYA_LIBRARIES}")
ENDIF()

