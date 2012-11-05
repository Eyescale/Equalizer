
# Copyright (c) 2012 Daniel Nachbaur <daniel.nachbaur@epfl.ch>
# provide SCM targets for status, untrack, reset and clean of source
# repositories. To be used in ExternalProject setups.

macro(SETUP_SCM name)
  get_property(cvs_repository TARGET ${name} PROPERTY _EP_CVS_REPOSITORY)
  get_property(svn_repository TARGET ${name} PROPERTY _EP_SVN_REPOSITORY)
  get_property(git_repository TARGET ${name} PROPERTY _EP_GIT_REPOSITORY)

  if(cvs_repository)
    set(SCM_STATUS ${CVS_EXECUTABLE} status)
    set(SCM_UNTRACK)
    set(SCM_RESET)
    set(SCM_RESET_FILE)
    set(SCM_CLEAN)
  elseif(svn_repository)
    set(SCM_STATUS ${Subversion_SVN_EXECUTABLE} st -q)
    set(SCM_UNTRACK)
    set(SCM_UNTRACK_FILE)
    set(SCM_RESET ${Subversion_SVN_EXECUTABLE} revert -R .)
    set(SCM_RESET_FILE ${Subversion_SVN_EXECUTABLE} revert)
    set(SCM_CLEAN)
  elseif(git_repository)
    set(SCM_STATUS ${GIT_EXECUTABLE} status --untracked-files=no -s)
    set(SCM_UNTRACK ${GIT_EXECUTABLE} reset -q)
    set(SCM_RESET ${GIT_EXECUTABLE} checkout -q -- .)
    set(SCM_RESET_FILE ${GIT_EXECUTABLE} checkout -q --)
    set(SCM_CLEAN ${GIT_EXECUTABLE} clean -qdxf)
  else()
    message(WARNING "Unknown repository type for ${name}")
  endif()
endmacro()
