# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

add_executable(eqHello MACOSX_BUNDLE hello.cpp)
target_link_libraries(eqHello Equalizer)
install(TARGETS eqHello DESTINATION "bin${EQ_INSTALL_POSTFIX}" COMPONENT apps)
install(FILES hello.cpp DESTINATION share/Equalizer/examples/eqHello COMPONENT examples)
