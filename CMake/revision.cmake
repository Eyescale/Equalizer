# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

find_package(Subversion)

if(Subversion_FOUND)
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE SVN_INFO_OUTPUT ERROR_VARIABLE SVN_INFO_ERROR)

  if(SVN_INFO_ERROR)
    message(STATUS "No revision version support, subversion error: ${SVN_INFO_ERROR}")
  else()
    Subversion_WC_INFO(${CMAKE_SOURCE_DIR} EQ_SVN)
    set(EQ_REVISION ${EQ_SVN_WC_REVISION})
    message(STATUS "  Revision ${EQ_REVISION}")
  endif()
else()
  message(STATUS "No revision version support, subversion not found")
endif(Subversion_FOUND)
