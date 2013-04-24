
# Copyright (c) 2012-2013 Stefan Eilemann <eile@eyescale.ch>

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
find_package(Git REQUIRED)
include(Doxygit)

foreach(FOLDER ${GIT_DOCUMENTATION_INSTALL})
  install(DIRECTORY ${FOLDER} DESTINATION share/${CMAKE_PROJECT_NAME})
endforeach()
install(FILES index.html DESTINATION share/${CMAKE_PROJECT_NAME})
