
# Copyright (c) 2012 Daniel Nachbaur <daniel.nachbaur@epfl.ch>
# provide SCM targets for status, untrack, reset and clean of source
# repositories. To be used in ExternalProject setups.

macro(SETUP_SCM name)
  get_property(svn_repository TARGET ${name} PROPERTY _EP_SVN_REPOSITORY)
  get_property(git_repository TARGET ${name} PROPERTY _EP_GIT_REPOSITORY)

  if(svn_repository)
    set(SCM_STATUS ${Subversion_SVN_EXECUTABLE} st -q)
    set(SCM_UNSTAGE)
    set(SCM_UNTRACK_FILE)
    set(SCM_RESET ${Subversion_SVN_EXECUTABLE} revert -R)
    set(SCM_CLEAN)
  elseif(git_repository)
    set(SCM_STATUS ${GIT_EXECUTABLE} status --untracked-files=no -s)
    set(SCM_UNSTAGE ${GIT_EXECUTABLE} reset -q)
    set(SCM_RESET ${GIT_EXECUTABLE} checkout -q --)
    set(SCM_CLEAN ${GIT_EXECUTABLE} clean -qdxf)
  else()
    message(WARNING "Unknown repository type for ${name}")
  endif()
endmacro()
