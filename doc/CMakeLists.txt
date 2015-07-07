# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

file(GLOB READMES . README*)
install(FILES ${READMES} FAQ Changelog.md ../AUTHORS
  ../CHANGES.md ../ACKNOWLEDGEMENTS ../LGPL ../LICENSE.txt
  DESTINATION share/Equalizer/doc COMPONENT doc)
