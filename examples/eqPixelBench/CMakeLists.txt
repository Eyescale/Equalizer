# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

add_executable(eqPixelBench MACOSX_BUNDLE
  channel.cpp
  config.cpp
  configEvent.cpp
  main.cpp
  window.cpp
  )

target_link_libraries(eqPixelBench Equalizer EqualizerAdmin)

install(TARGETS eqPixelBench DESTINATION bin)
