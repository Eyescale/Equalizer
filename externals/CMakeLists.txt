# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

file(GLOB HEADERS vmmlib/*.hpp)

forward_headers("" ${HEADERS})

install(FILES ${HEADERS} vmmlib/LICENSE
  DESTINATION include/vmmlib 
  COMPONENT vmmlib
  )
