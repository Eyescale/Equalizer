# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

file(GLOB READMES . README*)
install(FILES ${READMES} FAQ PLATFORMS RelNotes.md ../AUTHORS
  ../CHANGES.txt ../ACKNOWLEDGEMENTS ../LGPL ../LICENSE.txt
  DESTINATION share/Equalizer/doc COMPONENT doc)
