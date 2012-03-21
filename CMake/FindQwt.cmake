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

# QWT_FOUND - system has Qwt
# QWT_INCLUDE_DIR - where to find qwt.h
# QWT_LIBRARIES - the libraries to link against to use Qwt
# QWT_LIBRARY - where to find the Qwt library (not for general use)

if(NOT QT4_FOUND)
  include(FindQt4)
endif(NOT QT4_FOUND)

set(QWT_FOUND NO)

if(QT4_FOUND)
  find_path(QWT_INCLUDE_DIR qwt.h
    PATHS /usr/local/qwt /usr/local /usr/include/qwt-qt4 /usr/include/qwt
          /usr/include/qwt5 /usr /opt/local/include/qwt
    HINTS $ENV{QWT_ROOT} ${QWT_ROOT}
    PATH_SUFFIXES include
  )

  set(QWT_NAMES ${QWT_NAMES} qwt-qt4 qwt5 qwt libqwt-qt4 libqwt)
  find_library(QWT_LIBRARY
    NAMES ${QWT_NAMES}
    PATHS /usr/local/qwt /usr/local /usr $ENV{QWT_ROOT} ${QWT_ROOT}
    PATH_SUFFIXES lib
  )

  if(QWT_LIBRARY)
    set(QWT_LIBRARIES ${QWT_LIBRARY})
    set(QWT_FOUND YES)

  endif(QWT_LIBRARY)
endif(QT4_FOUND)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QWT DEFAULT_MSG QWT_LIBRARIES)

mark_as_advanced(QWT_INCLUDE_DIR QWT_LIBRARY)
