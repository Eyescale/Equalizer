#
#  The RealityGrid Steerer
#
#  Copyright (c) 2002-2009, University of Manchester, United Kingdom.
#  All rights reserved.
#
#  This software is produced by Research Computing Services, University
#  of Manchester as part of the RealityGrid project and associated
#  follow on projects, funded by the EPSRC under grants GR/R67699/01,
#  GR/R67699/02, GR/T27488/01, EP/C536452/1, EP/D500028/1,
#  EP/F00561X/1.
#
#  LICENCE TERMS
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#
#    * Neither the name of The University of Manchester nor the names
#      of its contributors may be used to endorse or promote products
#      derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
#  Author: Robert Haines

set(REG_DIR_DESCRIPTION "directory containing RealityGridConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/RealityGrid for an installation.")
set(REG_DIR_MESSAGE "RealityGrid not found.  Set the RealityGrid_DIR cmake cache entry to the ${REG_DIR_DESCRIPTION}")

#---------------------------------------------------------
# Search only if the location is not already known.
if(NOT RealityGrid_DIR)
  find_path(RealityGrid_DIR UseRealityGrid.cmake
    HINTS $ENV{STEER_LIB_ROOT} ${STEER_LIB_ROOT} $ENV{REG_STEER_HOME} $ENV{REG_HOME}}
    PATH_SUFFIXES lib/RealityGrid lib
    PATHS /usr /usr/local
    DOC "The ${REG_DIR_DESCRIPTION}"
  )
endif(NOT RealityGrid_DIR)

#---------------------------------------------------------

if(RealityGrid_DIR)
  # have we really found it?
  if(EXISTS ${RealityGrid_DIR}/RealityGridConfig.cmake)
    # yes - load settings
    set(RealityGrid_FOUND 1)
    include(${RealityGrid_DIR}/RealityGridConfig.cmake)
  else(EXISTS ${RealityGrid_DIR}/RealityGridConfig.cmake)
    # no
    set(RealityGrid_FOUND 0)
  endif(EXISTS ${RealityGrid_DIR}/RealityGridConfig.cmake)
else(RealityGrid_DIR)
  # not found
  set(RealityGrid_FOUND 0)
endif(RealityGrid_DIR)

#---------------------------------------------------------

if(NOT RealityGrid_FOUND)
  if(RealityGrid_FIND_REQUIRED)
    message(FATAL_ERROR ${REG_DIR_MESSAGE})
  else(RealityGrid_FIND_REQUIRED)
    if(NOT RealityGrid_FIND_QUIETLY)
      message(STATUS ${REG_DIR_MESSAGE})
    endif(NOT RealityGrid_FIND_QUIETLY)
  endif(RealityGrid_FIND_REQUIRED)
endif(NOT RealityGrid_FOUND)
