# Copied from OSG, with the following changes:
# - Sets OSG_LIBRARIES, which contains all libraries needed for eqOSG
# - Only sets OSG_FOUND to TRUE if all OSG libraries needed by eqOSG have been found
# - Looks for OSG in more places
# - Links to OpenThreads under Windows
# - Also looks for the library names with the 'd' prefix, for debug versions

# Locate gdal
# This module defines
# OSG_LIBRARY
# OSG_FOUND, if false, do not try to link to gdal 
# OSG_INCLUDE_DIR, where to find the headers
#
# $OSG_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OSG_DIR
#
# Created by Robert Osfield. 

FIND_PATH(OSG_INCLUDE_DIR osg/Node
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
    "C:/Programme/OpenSceneGraph/include"
    "C:/Program Files/OpenSceneGraph/include"
)

MACRO(FIND_OSG_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME} ${MYLIBRARYNAME}d
        PATHS
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
        "C:/Programme/OpenSceneGraph/lib"
        "C:/Program Files/OpenSceneGraph/lib"
    )

ENDMACRO(FIND_OSG_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSG_LIBRARY(OSG_LIBRARY osg)
FIND_OSG_LIBRARY(OSGUTIL_LIBRARY osgUtil)
FIND_OSG_LIBRARY(OSGDB_LIBRARY osgDB)
FIND_OSG_LIBRARY(OSGTEXT_LIBRARY osgText)
FIND_OSG_LIBRARY(OSGTERRAIN_LIBRARY osgTerrain)
FIND_OSG_LIBRARY(OSGFX_LIBRARY osgFX)
FIND_OSG_LIBRARY(OSGVIEWER_LIBRARY osgViewer)
FIND_OSG_LIBRARY(OSGGA_LIBRARY osgGA)

SET(OSG_LIBRARIES
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGGA_LIBRARY}
  ${OSGVIEWER_LIBRARY}
  ${OSGUTIL_LIBRARY}
)

FIND_OSG_LIBRARY(OSG_OPENTHREADS_LIBRARY OpenThreads)
SET(OSG_LIBRARIES ${OSG_OPENTHREADS_LIBRARY} ${OSG_LIBRARIES})

SET(OSG_FOUND "NO")
IF(OSG_LIBRARY AND OSGDB_LIBRARY AND OSGGA_LIBRARY AND OSGVIEWER_LIBRARY AND OSG_INCLUDE_DIR)
    SET(OSG_FOUND "YES")
ENDIF(OSG_LIBRARY AND OSGDB_LIBRARY AND OSGGA_LIBRARY AND OSGVIEWER_LIBRARY AND OSG_INCLUDE_DIR)
