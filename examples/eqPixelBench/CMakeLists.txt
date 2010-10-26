# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

set(HEADERS channel.h)

set(SOURCES
  channel.cpp
  config.cpp
  configEvent.cpp
  main.cpp
  window.cpp)

add_executable(eqPixelBench MACOSX_BUNDLE ${HEADERS} ${SOURCES})

target_link_libraries(eqPixelBench Equalizer EqualizerAdmin)

install(TARGETS eqPixelBench DESTINATION "bin${EQ_INSTALL_POSTFIX}" COMPONENT apps)
install(FILES ${HEADERS} ${SOURCES} DESTINATION share/Equalizer/examples/eqPixelBench COMPONENT examples)
