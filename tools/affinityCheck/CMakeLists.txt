# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

if(WIN32)
  set(AFFINITYCHECK_SOURCES affinityCheck.cpp)
  set(AFFINITYCHECK_LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})
  common_application(affinityCheck)
endif()
