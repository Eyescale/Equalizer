##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

function(PURPLE_INSTALL_PDB TARGET)
  if(MSVC)
    get_target_property(THIS_LOCATION ${TARGET} LOCATION)
	string(REPLACE "${CMAKE_CFG_INTDIR}" "\${BUILD_TYPE}" THIS_LOCATION ${THIS_LOCATION})
	get_filename_component(THIS_EXTENSION ${THIS_LOCATION} EXT)
	string(REGEX REPLACE "${THIS_EXTENSION}$" ".pdb" THIS_PDB ${THIS_LOCATION})
	install(FILES ${THIS_PDB} CONFIGURATIONS Debug RelWithDebInfo ${ARGN})
  endif(MSVC)
endfunction(PURPLE_INSTALL_PDB)
